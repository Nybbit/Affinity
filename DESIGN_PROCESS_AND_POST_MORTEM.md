# Design Process of Affinity and Post Mortem

Before reading this post mortem, I recommend trying Affinity out. You can get the installer for the latest version [here](https://github.com/Nybbit/Affinity/releases). In Affinity, you pilot your ship around and watch the red and green ships fight - it's a tech demo, not a triple A narrative experience. If you get caught in some cross fire and end up dying, you will go into "spectator mode" and the camera will just follow a random ship until one side wins. Then, the winning ships sail off to the east forever.

**Controls**

| Key | Description |
| --- | ----------- |
| W | Move forward |
| A | Rotate left |
| D | Rotate right |
| Up arrow | Zoom in |
| Down arrow | Zoom out |

### Why make Affinity?

I created Affinity because I wanted to make something that
- could handle thousands of entities at once that are doing their own thing
- was data-oriented and cache friendly
- used an entity component system
- was relatively easy to use

But most importantly, I created Affinity to learn new things.

## The Entity-Component-System (ECS)

This is the core of Affinity. In an Entity-Component-System you have entities, components, and systems that act on the entites and components.

**Entities**

An entity isn't special at all. In fact, in Affinity an entity is just an index. The *substance* of the entities comes from the...

**Components**

Components are just sets of data. Here's an example `Transform` component that holds position, scale and rotation data. It is 20 bytes large (this will be important later).

```cpp
struct Transform
{
    glm::vec2 position; // 8 bytes
    glm::vec2 scale;    // 8 bytes
    float rotation;     // 4 bytes
};
```

**Systems**

As some of you might have inferred by now, entities can have components. However, at this point, they're still just data and don't do anything. This is where systems come in. They act on entities with certain components.

**Example**

The best way to explain why ECS is so cool is to show off the modularity of it with an example.

Say we have a 3D multiplayer racing game with an ECS. Each of the cars would be an entity with components like a `Transform` (for positioning), `Model` (for the 3D model), and `Collision` (for collision detection and response). Then, all of the player-controlled cars could have a `PlayerControlled` (for converting user inputs into forces) component and all of the AI-controlled cars could have an `AiControlled` (that converts ai inputs into forces) component.

Here's where it gets cool. Say one of the players quits the game in a fit of rage mid-race. Now, we don't want the car to just sit in the middle of the road and nor do we want it to just poof out of existence. Instead, we can simply swap out the `PlayerControlled` component with the `AiControlled` component, and the car will now drive itself.

The systems are responsible for all the components working. For instance, the game might have a `RenderSystem` that goes through every entity with a `Transform` and `Model` component and render the model wherever the `Transform` component says to.

## Making the ECS Cache-Friendly and Data-Oriented Design

All of my game development projects up to this point have been Object-Oriented because that's how I learned to code. Unfortunately, OOP isn't always the best option for everything.

A little bit ago I started learning about data-oriented design. It's a different way of looking at a program that focuses on how you layout data in your program and how you work on it. (Here's a fantastic talk about [Data-Oriented Design](https://www.youtube.com/watch?v=rX0ItVEVjHc))

An object-oriented ECS might look something like this...

```cpp
struct Entity
{
    Transform transform;
    Sprite sprite;
    uint32_t bitmask;
};

struct ECS
{
    uint32_t numEntities;

    std::vector<Entity> entities;
};
```

while a data-oriented ECS might look something like this...

```cpp
struct ECS
{
    uint32_t numEntities;

    std::vector<Transform> transforms;
    std::vector<Sprite> sprites;
    std::vector<uint32_t> bitmasks;
};
```

See the difference? The object-oriented version is a Array of Structures (AOS) and the data-oriented version is a Structure of Arrays (SOA).

Let's compare cache-friendliness between the two with an example that moves all of the entities up by 1. First, with the object-oriented ECS...

```cpp
void moveAllEntitiesUp(ECS& ecs)
{
    for (uint32_t i = 0; i < ecs.numEntities; ++i)
    {
        if (ecs.entities[i].bitmask & TRANSFORM) // Check if entity has Transform component
        {
            ++ecs.entities[i].transform.position.y; // Move up
        }
    }
}
```

Each iteration, the entire entity is loaded (including the unused `Sprite` component). Now let's look at the same thing but with the data-oriented ECS...

```cpp
void moveAllEntitiesUp(ECS& ecs)
{
    for (uint32_t i = 0; i < ecs.numEntities; ++i)
    {
        if (ecs.bitmasks[i] & TRANSFORM) // Check if entity has Transform component
        {
            ++ecs.transforms[i].position.y; // Move up
        }
    }
}
```

Each iteration, only the parts of the entity that we want to check are loaded and there are more cache hits. Yay :tada:

Also, remember when I said that the size of the `Transform` component would be important later? This is why it's important. The smaller the size of the components, the more can fit in the cache line.

## A more versatile implementation

Now, something you may have noticed is that whenever we want to add a new component, we have to add it manually into the ECS class *and* add a bitflag. This can get annoying, especially since your ECS is most likely a little more complicated because it allows for destroying entities, reusing entities, enabling components, disabling components, etc. which means even more places that you have to manually add the component.

Now, a fairly obvious solution to this program that also doesn't increase the complexity too much is having the code instead be generated externally by a small python script or something. However, I decided *not* to do this because I wanted to see if I could instead do it without relying on code generation by a separate script. Ultimately, the code is harder to read and less performant that it could be with external code generation, but I got to get some good practice with templates and RTTI. Had this not been a learning project, I most likely would've opted for the first option.

## The Pirate Boat AI

This project was the first time I had ever done proper video-game "AI" and it turned out almost exactly how I wanted it to.

**AI Behavior States**

| State | Description |
| ----- | ----------- |
| APPROACH | Approach the target ship until the distance is less than or equal to `EFFECTIVE_RANGE * 2` (500 units) |
| ALIGN | Optimally, get perfectly parallel with the target but getting within a certain box and moving with the target is fine too. Shoot whenever the target is within an angle of `pi/4`. |
| WANDER | Go to the right, active when the AI has no target |

The thing I'm most unhappy with is how the code turned out. I'm sure that I could improve the code given the time and motivation as there are a few weird things that came out of me just trying to get it to work at 1 am (I believe somewhere in there I used a single variable for two different things? :scream:).

Functionally, the AI works pretty well. None of the boats coordinate with one another, this leads to a lot of friendly fire as well as some accidental strategic positioning.

## Spritesheet & Spritebatch

The spritesheet is based off of one of my other projects called [SpritesheetGenerator](https://github.com/Nybbit/SpritesheetGenerator) (creative name, I know). It's pretty fast and the way it's set up right now is it generates a new spritesheet every time the program is run and it doesn't save it. The ability to export and import it is there, but I didn't implement it because I only just now remembered.

The spritebatch is pretty simple. It batches a bunch of sprites to cut down on the amount of draw calls which makes rendering faster. An earlier version also used indices to cut down on the amount of vertices but it actually made rendering significantly worse because essentially none of the vertices were shared and it was just extra data *and* processing.

## If you were to do it again, what would you change?

**Entity-Component-System**

For getting components, I'd probably re-work the way that you access components. Currently, it's done like this...

```cpp
ecs.entityLoop([&](uint32_t i)
{
    const auto t  = static_cast<Transform*>(tVec[i]);
    const auto b  = static_cast<Boat*>(bVec[i]);

    if (b && t)
    {
        // if transform component and boat component
    }
});
```

but it would be really cool if instead it worked like this...

```cpp
ecs.entityLoop([&](uint32_t i, Transform& t, Boat& b)
{
    // if transform component and boat component
});
```

but I'd rather not delve into template hell.

Also, it would be cool to make it multi-threaded. A few of the systems there are embarassingly parallel.

**Templates**

As mentioned earlier, I'd probably generate the entity component system's class with a python script or something so that it doesn't rely on RTTI and a bunch of templates.

**Damaged boats**

Something you might have noticed is that there are more textures for ships than there are used. I was going to add damaged ships based on their health but I only now remembered I was going to do that. You can try adding it yourself if you want (it shouldn't be too hard)!

**Every todo**

There's a few TODOs scattered throughout the project of possible things to add. Implementing those would be neat but I've exhausted myself by working on this non-stop for the last bit.

**Spatial Partitioning**

Implementing spatial partitioning directly into the ECS might have been good to do to so that it is more accessible by systems. It could also be implemented in `BoatSystem` where the boats will try to find another target if their current target is invalid.

## Contact Me

This was fun, now I'm off to work on my next project, who knows what it will be?

Follow me on Twitter [@nybbit](https://twitter.com/nybbit)

Talk to me on Discord `@Nybbit#5412`

Email me at `nybbit[at]gmail.com`
