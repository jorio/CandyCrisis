# Candy Crisis source port change log

## 3.0.1 (June 2023)

- On non-Mac UNIXes, the game now looks for its assets in `(executable path)/../share/candycrisis` in addition to `(pwd)/CandyCrisisResources`. This is meant to help Linux distros package the game.
- Most file/string operations were delegated to SDL instead of libc.
- Fixed minor memory leak.

## 3.0.0 (February 2023)

Initial release of my unofficial port of Candy Crisis.
