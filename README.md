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

Host

- OS: macOS 14.7
- UTM 4.6.5
- Ruby 3.3.0
- Vagrant 2.4.9
- vagrant_utm 0.1.3 (Version Constraint: > 0)
- [vagrant-bindfs](https://github.com/gael-ian/vagrant-bindfs) 1.3.1

VM for development

- OS: Ubuntu 22.04
- Rust 1.89.0stable (at 2025/06/23)

## Setup

### Install UTM

See [UTM homepage](https://mac.getutm.app/).

For macOS users, this article is helpful (only in Japanese): [【macOS】UTM で Ubuntu Desktop 24.04 LTS の仮想マシン環境を構築する](https://zenn.dev/pyteyon/scraps/0c8cec3c56812b)

### Install Vagrant

```sh
brew install vagrant

# install vagrant plugins
vagrant plugin install vagrant_utm vagrant-bindfs
```

### Setup VM

```sh
cd toy-tcpip-rs

# start VM
vagrant up

# start VM with provision
vagrant up --provision

# reload VM
vagrant reload

# realod VM with provision
vagrant reload --provision

# status VM
vagrant status

# ssh to VM
vagrant ssh

# get SSH config (e.g. for VSCode Remote-SSH)
vagrant ssh-config

# stop VM
vagrant halt
```

Now, you can start

## References

- [github.com/pandax381/microps](https://github.com/pandax381/microps)
  - Implementation of TCP/IP protocol stack in C. This is a reference implementation of this project.
- [KLab Expert Camp 6 - reference materials](https://drive.google.com/drive/folders/1k2vymbC3vUk5CTJbay4LLEdZ9HemIpZe)
  - A series of lectures on TCP/IP protocol stack, [microps](https://github.com/pandax381/microps). This project is based on the contents of this lecture.
- [Available Vagrant boxes for UTM](https://portal.cloud.hashicorp.com/vagrant/discover?query=utm)
  - Vagrant plugin for UTM.

## License

Copyright (c) 2012-2021 YAMAMOTO Masaya

toy-tcpip is licensed under the MIT License. For more details, check out the [LICENSE](./LICENSE) file.
