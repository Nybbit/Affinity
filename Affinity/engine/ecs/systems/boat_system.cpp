#include "boat_system.h"
#include "../components.h"
#include "../../engine.h"
#include "../../config.h"

void BoatSystem::update(Engine& engine, EntityComponentSystem& ecs)
{
	auto& tVec  = ecs.getComponentVector<Transform>();
	auto& bVec  = ecs.getComponentVector<Boat>();
	auto& aiVec = ecs.getComponentVector<BoatAi>();

	const auto time = static_cast<int>(glfwGetTime() / 5.0f);

	ecs.entityLoop([&](uint32_t i)
	{
		const auto t  = static_cast<Transform*>(tVec[i].get());
		const auto b  = static_cast<Boat*>(bVec[i].get());
		const auto ai = static_cast<BoatAi*>(aiVec[i].get());

		if (b && t)
		{
			// Handle cooldown for cannons
			if (b->cooldownRemaining > 0.0f)
			{
				b->cooldownRemaining -= static_cast<float>(engine.getFrameTimer().getDelta());
				if (b->cooldownRemaining < 0.0f)
				{
					b->cooldownRemaining = 0.0f;
				}
			}

			// If there's an AI component, do ai stuff
			if (ai)
			{
				handleAi(engine, ecs, *b, *t, *ai, i);
			}
		}
	});
}

bool BoatSystem::shouldFire(float dot_product)
{
	return abs(dot_product) > glm::quarter_pi<float>();
}

std::pair<glm::vec2, glm::vec2> BoatSystem::findContactsFromTangent(glm::vec2 circle_center, float radius,
                                                                    float tangent_slope) const
{
	const auto rSlope = -1.0f / tangent_slope;

	const auto diff = glm::vec2(cos(rSlope), sin(rSlope)) * radius;

	const auto p1 = circle_center + diff;
	const auto p2 = circle_center - diff;

	return std::make_pair(p1, p2);
}

glm::vec2 BoatSystem::getTangentVec(glm::vec2 circle_center, glm::vec2 point) const
{
	const auto vec = circle_center - point;
	return glm::vec2(vec.y, -vec.x);
}

// todo: rewrite this function, too many fmods and stuff
void BoatSystem::rotateBoatTowardVec(Engine& engine, float& rotation, glm::vec2 target_vec) const
{
	const auto rotationSpeed = config::ROTATION_SPEED * static_cast<float>(engine.getFrameTimer().getDelta());

	const auto targetRot = atan2(target_vec.y, target_vec.x);

	// Get the smallest difference for rotation
	auto rotDiff = fmod(targetRot - rotation, glm::two_pi<float>());

	// Can quit early if there's no rotation difference
	if (rotDiff == 0)
	{
		return;
	}

	// Prefer small rotations over large rotations
	if (abs(rotDiff) > glm::pi<float>())
	{
		rotDiff = -fmod(rotDiff, glm::pi<float>());
	}

	// Rotate
	const auto rotChange = abs(rotDiff) <= rotationSpeed
		                       ? rotDiff
		                       : (rotDiff < 0 ? -rotationSpeed : rotationSpeed);
	rotation = fmod(rotation + rotChange, glm::two_pi<float>());
}

void BoatSystem::handleAi(Engine& engine, EntityComponentSystem& ecs, Boat& b, Transform& t, BoatAi& ai,
                          uint32_t entity_index)
{
	if (ai.target.isValid())
	{
		if (const auto targetT = ai.target.getComponent<Transform>())
		{
			const auto distance = glm::distance(targetT->position, t.position);

			if (distance > EFFECTIVE_RANGE * 2)
			{
				ai.behavior = BoatAi::BehaviorEnum::APPROACH;
				performApproachAi(engine, t, b, ai);
			}
			else
			{
				ai.behavior = BoatAi::BehaviorEnum::ALIGN;
				performAlignAi(engine, ecs, t, b, ai, entity_index);
			}
		}
	}
	else
	{
		ai.behavior = BoatAi::BehaviorEnum::WANDER;
		performWanderAi(engine, t, b);
		findTarget(engine, ecs, b, t, ai);
	}
}

bool BoatSystem::isBoatInOptimalBox(Transform& t, Transform& target_t)
{
	const auto optimalBox = glm::vec4(
		glm::vec2(-target_t.scale.x * 0.5f, -EFFECTIVE_RANGE),
		target_t.scale.x,
		EFFECTIVE_RANGE * 2);

	const auto s            = sin(-target_t.rotation);
	const auto c            = cos(-target_t.rotation);
	const auto point        = t.position - target_t.position;
	const auto rotatedPoint = glm::vec2(point.x * c - point.y * s,
	                                    point.x * s + point.y * c);

	return !(rotatedPoint.x < optimalBox.x || rotatedPoint.y < optimalBox.y || rotatedPoint.x >
	         optimalBox.x + optimalBox.z || rotatedPoint.y > optimalBox.y + optimalBox.w);
}

void BoatSystem::moveBoatForward(Engine& engine, Transform& t, Boat& b)
{
	// Get the current vec based on rotation
	const auto currentVec = glm::vec2(cos(t.rotation), sin(t.rotation));

	// Move in facing direction
	t.position += currentVec * b.speed
		* static_cast<float>(engine.getFrameTimer().getDelta());
}

void BoatSystem::performWanderAi(Engine& engine, Transform& t, Boat& b)
{
	const auto targetVec = glm::vec2(1, 0); // todo: randomize

	rotateBoatTowardVec(engine, t.rotation, targetVec == glm::vec2(0) ? glm::vec2(1) : targetVec);

	// Speed is half of max speed because if you're wandering around you're probably not in a rush
	b.speed = b.maxSpeed * 0.5f;

	moveBoatForward(engine, t, b);
}

void BoatSystem::performApproachAi(Engine& engine, Transform& t, Boat& b, BoatAi& ai)
{
	const auto targetPos = ai.target.getComponent<Transform>()->position;
	const auto targetVec = targetPos - t.position;

	// Rotate the boat to the target vec
	rotateBoatTowardVec(engine, t.rotation, targetVec);

	// Get the current vec based on rotation
	const auto currentVec = glm::vec2(cos(t.rotation), sin(t.rotation));

	// Set speed according to how similar the current vec is to the target vec
	b.speed = glm::max(0.0f, glm::dot(glm::normalize(targetVec), currentVec))
	          * b.maxSpeed;

	moveBoatForward(engine, t, b);
}

// Sorry, there's some ugly code here, todo: clean up
void BoatSystem::performAlignAi(Engine& engine, EntityComponentSystem& ecs, Transform& t, Boat& b, BoatAi& ai,
                                uint32_t entity_index)
{
	const auto targetT     = ai.target.getComponent<Transform>();
	const auto targetSpeed = ai.target.getComponent<Boat>()->speed;

	auto tangentVec = glm::vec2(cos(targetT->rotation), sin(targetT->rotation));

	// Contacts are optimal points to be at
	const auto contacts = findContactsFromTangent(
		targetT->position + glm::vec2(cos(targetT->rotation), sin(targetT->rotation)) / glm::
		distance(t.position, targetT->position),
		EFFECTIVE_RANGE, tangentVec.y / tangentVec.x);

	auto targetPos = contacts.first;
	auto dist      = glm::distance(contacts.first, t.position);

	if (glm::distance(contacts.second, t.position) < dist)
	{
		dist      = glm::distance(contacts.second, t.position);
		targetPos = contacts.second;
	}

	// Get the vector to be at so that the boat is parallel to target
	tangentVec = getTangentVec(targetT->position, t.position);
	{
		const auto targetBoatVec   = glm::vec2(cos(targetT->rotation), sin(targetT->rotation));
		const auto otherTangentVec = -glm::vec2(tangentVec.x, tangentVec.y);
		if (abs(glm::dot(tangentVec, targetBoatVec)) > abs(glm::dot(otherTangentVec, targetBoatVec))
		)
		{
			tangentVec = otherTangentVec;
		}
	}

	// Optimal box to be in for shooting at target
	bool inOptimalBox = isBoatInOptimalBox(t, *targetT);

	auto targetVec = inOptimalBox ? tangentVec : (targetPos - t.position);

	rotateBoatTowardVec(engine, t.rotation, targetVec);

	// Get the current vec based on rotation
	const auto currentVec = glm::vec2(cos(t.rotation), sin(t.rotation));

	// Set speed according to how similar the current vec is to the target vec
	b.speed = targetVec == tangentVec
		          ? glm::min(b.maxSpeed, targetSpeed)
		          : glm::max(0.0f, glm::dot(glm::normalize(targetVec), currentVec))
		            * b.maxSpeed;

	moveBoatForward(engine, t, b);

	// If can fire
	if (b.cooldownRemaining == 0.0f)
	{
		const auto dotProduct = glm::dot(currentVec, glm::normalize(tangentVec));

		if (shouldFire(dotProduct))
		{
			const auto shouldShootFromRightSide = dotProduct >= 0;

			const auto ballVec = glm::vec2(
				cos(t.rotation + glm::half_pi<float>() * (shouldShootFromRightSide ? 1 : -1)),
				sin(t.rotation + glm::half_pi<float>() * (shouldShootFromRightSide ? 1 : -1)));

			auto ball = ecs.createEntity();
			ball.setComponent<Transform>(
				Transform(t.position + ballVec * t.scale.y * 0.5f, atan2(ballVec.y, ballVec.x),
				          config::CANNONBALL_SIZE));
			ball.setComponent<Sprite>(
				Sprite(engine.getRenderer().getSpritesheet().getUv(
					"cannonBall")));
			ball.setComponent<CannonBall>(CannonBall(ballVec, 30, ecs.getEntityByIdx(entity_index)));

			b.cooldownRemaining = 60.0f;
		}
	}
}

void BoatSystem::findTarget(Engine& engine, EntityComponentSystem& ecs, Boat& b, Transform& t, BoatAi& ai)
{
	auto& bVec = ecs.getComponentVector<Boat>();
	auto& tVec = ecs.getComponentVector<Transform>();

	float dist         = -1;
	uint32_t targetIdx = 0;

	// todo: spatial partitioning
	ecs.entityLoop([&](uint32_t i)
	{
		if (const auto boat = static_cast<Boat*>(bVec[i].get()))
		{
			const auto targetT = static_cast<Transform*>(tVec[i].get());
			if (targetT && boat->team != Boat::BoatTeamEnum::NEUTRAL && boat->team != b.team)
			{
				const auto targetDist = glm::distance(t.position, targetT->position);
				if (dist == -1 || targetDist < dist)
				{
					dist      = targetDist;
					targetIdx = i;
				}
			}
		}
	});

	if (dist != -1)
	{
		ai.target = ecs.getEntityByIdx(targetIdx);
	}
}
