# Ray Tracing in a Weekend

Following along with the [Ray Tracing in One Weekend](https://raytracing.github.io/books/RayTracingInOneWeekend.html) book.

The book gives step by step instructions to write a ray tracer in C++. In this repo I follow the steps but I am using C instead of C++. The output of the program is a [PPM image file](https://en.wikipedia.org/wiki/Netpbm).

```
make
./main > image.ppm
```

For more information about this repository, see the [blog post](https://blog.jacobvosmaer.nl/0022-ray-tracing-weekend/) where I talk about what I did here.

In some of the branches of this repository I experimented with various speed optimizations. This is not the point of the Ray Tracing book but I thought it was fun. Also see these posts ([1](https://www.jakef.science/posts/gcc-wtf/), [2](https://www.jakef.science/posts/recursion-riddle/) and [3](https://www.jakef.science/posts/simd-parallelism/)) by fellow Recurse participant Jake who was exploring this stuff at the same time as me and who did a great job writing about it.
