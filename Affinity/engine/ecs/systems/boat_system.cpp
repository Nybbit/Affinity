#include "boat_system.h"
#include "../components.h"
#include "../../engine.h"
#include "../../config.h"

void BoatSystem::update(Engine& engine, EntityComponentSystem& ecs)
{
	auto& tVec  = ecs.getComponentVector<Transform>();
	auto& bVec  = ecs.getComponentVector<Boat>();
	auto& aiVec = ecs.getComponentVector<BoatAi>();

	ecs.entityLoop([&](uint32_t i)
	{
		const auto t  = static_cast<Transform*>(tVec[i].get());
		const auto b  = static_cast<Boat*>(bVec[i].get());
		const auto ai = static_cast<BoatAi*>(aiVec[i].get());

		// Boat and transform components are required
		if (!(b && t))
		{
			return;
		}

		// Handle cooldown for cannon firing
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
	});
}

std::pair<glm::vec2, glm::vec2> BoatSystem::findContactsFromTangent(glm::vec2 circle_center, float radius,
                                                                    float tangent_slope) const
{
	// Get perpendicular slope
	const auto rSlope = -1.0f / tangent_slope;

	// Vector of length radius in direction of rSlope
	const auto diff = glm::vec2(cos(rSlope), sin(rSlope)) * radius;

	// Find the two contacts
	const auto p1 = circle_center + diff;
	const auto p2 = circle_center - diff;

	// Return them as a pair
	return std::make_pair(p1, p2);
}

glm::vec2 BoatSystem::getTangentVec(glm::vec2 circle_center, glm::vec2 point) const
{
	// Get the vector towards the circle center
	const auto vec = circle_center - point;

	// Return perpendicular
	return glm::normalize(glm::vec2(vec.y, -vec.x));
}

// todo: rewrite this function, too many fmods and stuff
void BoatSystem::rotateBoatTowardVec(Engine& engine, float& rotation, glm::vec2 target_vec) const
{
	const auto rotationSpeed = config::ROTATION_SPEED * static_cast<float>(engine.getFrameTimer().getDelta());

	// Convert vector to angle
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
		// Target needs to have a transform component
		if (const auto targetT = ai.target.getComponent<Transform>())
		{
			// Get the distance between the two - todo: replace distance func, unnecessary sqrt
			const auto distance = glm::distance(targetT->position, t.position);

			// Depending on distance, choose a behavior. If neutral, always wander
			if (distance > EFFECTIVE_RANGE * 2 && b.team != Boat::BoatTeamEnum::NEUTRAL)
			{
				// Get closer to target
				ai.behavior = BoatAi::BehaviorEnum::APPROACH;
				performApproachAi(engine, t, b, ai);
			}
			else
			{
				// Align with target and fire when possible
				ai.behavior = BoatAi::BehaviorEnum::ALIGN;
				performAlignAi(engine, ecs, t, b, ai, entity_index);
			}
		}
	}
	else
	{
		// If the current target is an invalid entity, wander and try to find a new target
		ai.behavior = BoatAi::BehaviorEnum::WANDER;
		performWanderAi(engine, t, b);
		findTarget(engine, ecs, b, t, ai);
	}
}

bool BoatSystem::isBoatInOptimalBox(Transform& t, Transform& target_t)
{
	/*
	 Optimal box goes out from sides of boat like so

		 +----+
		 |    |
		 [BOAT]>
		 |    |
		 +----+
	 */

	const auto optimalBox = glm::vec4(
		-target_t.scale.x * 0.5f, // x
		-EFFECTIVE_RANGE,         // y
		target_t.scale.x,         // width
		EFFECTIVE_RANGE * 2       // height
	);

	// Transform boat coordinate into optimal box coordinate plane
	const auto s            = sin(-target_t.rotation);
	const auto c            = cos(-target_t.rotation);
	const auto point        = t.position - target_t.position;
	const auto rotatedPoint = glm::vec2(point.x * c - point.y * s,
	                                    point.x * s + point.y * c);

	// Check if point is in optimal box
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

// This code is a little complicated, hopefully it's not too hard to get a sense of what's happening
void BoatSystem::performAlignAi(Engine& engine, EntityComponentSystem& ecs, Transform& t, Boat& b, BoatAi& ai,
                                uint32_t entity_index)
{
	const auto targetT     = ai.target.getComponent<Transform>();
	const auto targetSpeed = ai.target.getComponent<Boat>()->speed;

	// Get the vector to be at so that the boat is parallel to target
	auto tangentVec = getTangentVec(targetT->position, t.position);

	// Make sure that the tangent vector is going in generally the same direction as the target
	{
		const auto targetBoatVec = glm::vec2(cos(targetT->rotation), sin(targetT->rotation));
		const auto oppositeTangentVec = -glm::vec2(tangentVec.x, tangentVec.y);
		if (abs(glm::dot(tangentVec, targetBoatVec)) > abs(glm::dot(oppositeTangentVec, targetBoatVec)))
		{
			tangentVec = oppositeTangentVec;
		}
	}

	// Optimal box to be in for shooting at target
	const auto inOptimalBox = isBoatInOptimalBox(t, *targetT);

	// If in optimal box, target vec is tangent vec
	auto targetVec = tangentVec;

	if (!inOptimalBox)
	{
		// Contacts are optimal points to be at for attacking
		const auto contacts = findContactsFromTangent(
			targetT->position + glm::vec2(cos(targetT->rotation), sin(targetT->rotation)) / glm::
			distance(t.position, targetT->position),
			EFFECTIVE_RANGE, sin(targetT->rotation) / cos(targetT->rotation));

		// Find nearest contact
		auto targetPos = contacts.first;
		auto dist      = glm::distance(contacts.first, t.position);
		if (glm::distance(contacts.second, t.position) < dist)
		{
			dist      = glm::distance(contacts.second, t.position);
			targetPos = contacts.second;
		}

		// Set target vec towards target pos
		targetVec = targetPos - t.position;
	}

	// Adjust rotation towards target vector
	rotateBoatTowardVec(engine, t.rotation, targetVec);

	// Get the current vec based on rotation
	const auto currentVec = glm::vec2(cos(t.rotation), sin(t.rotation));

	if (inOptimalBox)
	{
		// Try to match speed with target
		b.speed = glm::min(b.maxSpeed, targetSpeed);
	}
	else
	{
		// Set speed based on how close boat's rotation is to target vec
		b.speed = glm::max(0.0f, glm::dot(currentVec, glm::normalize(targetVec))) * b.maxSpeed;
	}

	// Get the dot product of the current vec and tangent vec
	const auto dotProduct = glm::dot(currentVec, tangentVec);

	// If can fire
	if (b.cooldownRemaining == 0.0f)
	{
		// If should fire
		if (abs(dotProduct) > glm::quarter_pi<float>())
		{
			// Reset cooldown to 60 ticks
			b.cooldownRemaining = 60.0f;

			// Figure out ball vec
			const auto shouldShootFromRightSide = dotProduct >= 0;
			const auto ballVec = glm::vec2(
				cos(t.rotation + glm::half_pi<float>() * (shouldShootFromRightSide ? 1 : -1)),
				sin(t.rotation + glm::half_pi<float>() * (shouldShootFromRightSide ? 1 : -1)));

			// Create the cannonball
			auto ball = ecs.createEntity();
			ball.setComponent<Transform>(
				Transform(t.position + ballVec * t.scale.y * 0.5f, atan2(ballVec.y, ballVec.x),
				          config::CANNONBALL_SIZE));
			ball.setComponent<Sprite>(
				Sprite(engine.getRenderer().getSpritesheet().getUv(
					"cannonball")));
			ball.setComponent<Cannonball>(Cannonball(ballVec, 30, ecs.getEntityByIdx(entity_index)));
		}
	}

	// Move the boat in the direction it's facing
	moveBoatForward(engine, t, b);
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
		// Target must be a boat
		if (const auto boat = static_cast<Boat*>(bVec[i].get()))
		{
			// Target must be on opposite team
			const auto targetT = static_cast<Transform*>(tVec[i].get());
			if (targetT && boat->team != Boat::BoatTeamEnum::NEUTRAL && boat->team != b.team)
			{
				// Check if closer than current closest
				const auto targetDist = glm::distance(t.position, targetT->position);
				if (dist == -1 || targetDist < dist)
				{
					dist      = targetDist;
					targetIdx = i;
				}
			}
		}
	});

	// If a new target was found, set the target
	if (dist != -1)
	{
		ai.target = ecs.getEntityByIdx(targetIdx);
	}
}
