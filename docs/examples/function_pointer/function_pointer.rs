type NetFn = fn(&NetDevice);

struct NetDevice {
    name: String,
    open: NetFn,
    close: NetFn,
}

fn open_device(dev: &NetDevice) {
    println!("open {}", dev.name);
}

fn close_device(dev: &NetDevice) {
    println!("close {}", dev.name);
}

fn main() {
    println!("Hi, Rust!");

    let dev = NetDevice {
        name: "eth0".to_string(),
        open: open_device,
        close: close_device,
    };

    (dev.open)(&dev);
    (dev.close)(&dev);
}
