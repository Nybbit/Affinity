> **Note:** This code no longer meets my standards of quality, rather than delete it I'll keep it around as a "code time capsule". Enjoy! :smile:

![Affinity Banner](https://i.imgur.com/OFs5Nmh.png)

## What is Affinity?

**Affinity** is a tech-demo and also a personal learning project. I would **deeply** appreciate any constructive criticism as to how I can improve my code and why it's an improvement.

## [Read about the design process and post mortem here](https://github.com/Nybbit/Affinity/blob/master/DESIGN_PROCESS_AND_POST_MORTEM.md)

### Key features

- Performant data-oriented entity component system
- AI pirate ships
- Fast spritesheet generation with texture packing

### Benchmarks

> **NOTE:** The benchmark is of an earlier (simpler) version of Affinity, it consists of 10,000 boats wandering around and various zoom levels. [You can try it out yourself here.](https://github.com/Nybbit/Affinity/releases/tag/v0.0.1-benchmark) The benchmarks shown below are not all of the benchmarks I recieved but should be representative of the large majority of PCs.

| CPU | GPU | Rounded Average Frametime (µs) | Rounded Average FPS | Notes |
| --- | --- | ------------------------------ | ------------------- | ----- |
| Intel(R) Core(TM) i7-8700 CPU @ 3.20GHz | NVIDIA GeForce GTX 1060 6GB | 3,164 | 384 | Notice the lower CPU clock speed but higher performance when compared to the next benchmark, this CPU is newer and more efficient - clock speed isn't everything |
| Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz | NVIDIA GeForce GTX 1060 6GB | 4,180 | 309 | Faster clock speed, older cpu, worse performance |
| AMD FX(tm)-8350 Eight-Core Processor | GeForce GTX 650 Ti | 5,641 | 221 | This CPU prefers more cores which unfortunately doesn't help it in the benchmark since the benchmark is single threaded |
| Intel(R) Core(TM) i3-8130U CPU @ 2.20GHz | Intel(R) UHD Graphics 620 | 5,337 | 223 | A fairly standard laptop |
| AMD A4-3300M APU with Radeon(tm) HD Graphics | AMD Radeon HD 6480G | 34,309 | 31 | This is a 9 year old laptop that was not made for gaming or anything |

### Third Party

- Pirate assets by [Kenney](https://kenney.nl/assets/pirate-pack)
- [GLFW](https://www.glfw.org/)
- [GLM](https://glm.g-truc.net)
- [GLEW](http://glew.sourceforge.net/)
- [Plog](https://github.com/SergiusTheBest/plog)
- [stb](https://github.com/nothings/stb)
- [ImGui](https://github.com/ocornut/imgui)
