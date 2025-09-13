$vm_name = "toy-tcpip"
$vm_cpus = 4
$vm_memory = 4096
$vm_notes = "VM for toy-tcpip"
$vm_directory_share_mode = "virtFS"
$vm_forwarded_port = 80
$vm_host_port = 8080

# for synced folder
$base_dir = "/home/vagrant"
$sync_dir = "#{$base_dir}/_mnt_toy-tcpip-rs"
$work_dir = "#{$base_dir}/toy-tcpip-rs"

# ref: https://naveenrajm7.github.io/vagrant_utm/configuration.html
Vagrant.configure("2") do |config|
  # ref: https://portal.cloud.hashicorp.com/vagrant/discover/utm/ubuntu-24.04
  config.vm.box = "utm/ubuntu-24.04"
  config.vm.box_version = "0.0.1"

  # Hostname inside the VM
  config.vm.hostname = $vm_name

  # Ports to forward
  # localhost:80 access is forwarded to <vm>:8080
  config.vm.network "forwarded_port", guest: $vm_forwarded_port, host: $vm_host_port

  # Synced folder
  config.vm.synced_folder ".", $sync_dir, create: true

  # 501:20 → 1000:1000 に見せる
  config.bindfs.bind_folder $sync_dir, $work_dir,
    map: "501/1000:@20/@1000", perms: "u=rwX:g=rwX:o=rX", create_as_user: true

  # Provider specific configs
  config.vm.provider "utm" do |u|
    # Name in UTM UI
    u.name = $vm_name

    # Machine type
    # not supported on vagrant_utm plugin
    # u.machine = "vf"

    # CPU in cores
    u.cpus = $vm_cpus

    # Memory in MB
    u.memory = $vm_memory

    # Notes for UTM VM (Appears in UTM UI)
    u.notes = $vm_notes

    # QEMU Directoy Share mode for the VM.
    # Takes none, webDAV or virtFS
    u.directory_share_mode = $vm_directory_share_mode
  end

  # Provisioner config, supports all built provisioners
  # ref: https://developer.hashicorp.com/vagrant/docs/provisioning/shell
  config.vm.provision "shell", privileged: false, path: "scripts/bootstrap.sh"
end
