# [WIP] toy-tcpip

[![MIT license badge][mit-badge]][mit-url]
[![GitHub Actions workflow badge][github-actions-badge]][github-actions-url]

[mit-badge]: https://img.shields.io/badge/license-MIT-blue.svg
[mit-url]: https://github.com/nukopy/toy-tcpip/blob/main/LICENSE
[github-actions-badge]: https://github.com/nukopy/toy-tcpip/actions/workflows/ci.yml/badge.svg?branch=main
[github-actions-url]: https://github.com/nukopy/toy-tcpip/actions/workflows/ci.yml?query=branch:main

Toy TCP/IP protocol stack written in Rust

This project is a Rust implementation of a TCP/IP protocol stack written in C, [microps](https://github.com/pandax381/microps).

## Features

TODO

ref: https://github.com/pandax381/microps#features

## Environment

Developlment environment is based on Docker Compose.

Container environment is as follows:

- OS: Ubuntu 20.04
- Rust 1.71.1 stable (at 2023/08/11)

For more details, check out the [Dockerfile](./Dockerfile) and [docker-compose.yml](./docker-compose.yml).

## Environment Setup

### Requirements

- Docker Compose (latest)

### Develop in Container

#### (Recommended) on Visual Studio Code with "Dev Containers"

1. Open this project on Visual Studio Code (VS Code)
2. Create workspace file like `toy-tcpip.code-workspace`
3. Run VSCode command (`cmd + shift + P` keybind on macOS): `Dev Containers: Rebuild Container Without Cache`
4. Start developing!

#### on Terminal

```sh
# in host
docker compose up -d
docker compose exec dev bash # "dev" is service name in docker-compose.yml

# in container
# start developing!
```

## References

- [github.com/pandax381/microps](https://github.com/pandax381/microps)
  - Implementation of TCP/IP protocol stack in C. This is a reference implementation of this project.
- [KLab Expert Camp 5](https://drive.google.com/drive/folders/1k2vymbC3vUk5CTJbay4LLEdZ9HemIpZe)
  - A series of lectures on TCP/IP protocol stack, [microps](https://github.com/pandax381/microps). This project is based on the contents of this lecture.

## License

Copyright (c) 2012-2021 YAMAMOTO Masaya

toy-tcpip is licensed under the MIT License. For more details, check out the [LICENSE](./LICENSE) file.
