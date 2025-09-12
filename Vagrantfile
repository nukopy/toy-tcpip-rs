$vm_name = "toy-tcpip"
$vm_cpus = 4
$vm_memory = 4096
$vm_notes = "VM for toy-tcpip"
$vm_directory_share_mode = "virtFS"
$vm_forwarded_port = 80
$vm_host_port = 8080

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
  # 例: プロジェクトを /home/vagrant/toy-tcpip-rs に共有（typeは書かない＝UTMが9pで付ける）
  # マウント用のディレクトリ。ホスト ./toy-tcpip-rs が /home/vagrant/toy-tcpip-rs_sync にマウントされる。
  # このディレクトリは UID / GID が 501:20 のため、VM 上で直接は読み書き実行できない。
  $sync_dir = "/home/vagrant/_mnt_toy-tcpip-rs"
  config.vm.synced_folder ".", $sync_dir, create: true

  # 501:20 → 1000:1000 に見せ替える “書けるビュー”
  # 実際のワーキングディレクトリ
  $work_dir = "/home/vagrant/toy-tcpip-rs"
  config.bindfs.bind_folder $sync_dir, $work_dir,
    map: "501/1000:@20/@1000", perms: "u=rwX:g=rwX:o=rX", create_as_user: true

  # config.vm.synced_folder ".", "/home/vagrant/toy-tcpip-rs",
  #   # type: "nfs",
  #   # nfs_udp: false,
  #   # mount_options: ["rw", "vers=4", "tcp"],
  #   # map_uid: Process.uid,
  #   # map_gid: Process.gid,
  #   # create: true
  #   # type: "virtiofs",
  #   # Vagrant UTM プラグインでは default で UTM QEMU VirtFS を同期フォルダの実装として使用している。
  #   # ref: https://naveenrajm7.github.io/vagrant_utm/features/synced_folders.html
  #   owner: "vagrant",
  #   group: "vagrant",
  #   create: true

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
  # shell, ansible
  # ref: https://developer.hashicorp.com/vagrant/docs/provisioning/shell
  config.vm.provision "shell", path: "scripts/bootstrap.sh"

  # config.vm.provision "shell", inline: <<-SHELL
  # # apt-get update
  # echo "Hello, World!"
  # SHELL
end
