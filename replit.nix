{ pkgs }: {
	deps = [
		pkgs.jq.bin
  pkgs.unzip
  pkgs.clang_12
		pkgs.ccls
		pkgs.gdb
		pkgs.gnumake
	];
}