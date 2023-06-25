{ pkgs }: {
	deps = [
		pkgs.cmake
  pkgs.jq.bin
  pkgs.unzip
  pkgs.clang_12
		pkgs.ccls
		pkgs.gdb
		pkgs.gnumake
	];
}