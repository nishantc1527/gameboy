# The Rust Experiment

I am considering porting large parts of the emulator to Rust. If I plan to do native unit testing or add more advanced features like network connections or accelerated rendering, it might be easier to use a modern safe language like Rust. But I'm not convinced yet, so I decided to port a portion of the emulator to Rust and see how it goes.

I chose the MMU for a few reasons:

1. The largest heap allocations are done here, so it's the most important part to get right memory wise
1. Exhaustive pattern matching is really nice for a lot of MMU features, like validating RAM sizes
1. Fearless file IO operations
1. MMU should be isolated and a black box, so I don't have to mess with bindgen stuff
1. The MMU had the most potential error points, so it's nice to be forced to handle all of those

# Results

I ran into a pretty nasty bug that took me a few days to fix ([the bug](https://github.com/nishantc1527/gameboy/commit/f40c48f20e5173f21f7f48699635c69c20f35eb4)). It came from the fact that Rust is pretty explicit about doing operations on values with different sizes, and it wants all indicies to be of size `usize`. Honestly I'm not sure if this is a skill issue or a Rust issue, and I'm leaning towards skill issue. But I know this error wouldn't come in C because I can add whatever I want together. Whether that's a good thing or not is subjective.

All things considered Rust was very nice to work with. I liked not having to worry about which new gigantic blocks of memory I'm allocating, I liked not having to worry about having an unhandled error, and the pattern matching was super nice. For most future features or patches I'll probably just stick to C but I'll consider Rust for larger or more error prone features in the future.
