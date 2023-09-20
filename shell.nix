{ pkgs ? import <nixpkgs> {} }: pkgs.mkShell {
  name = "rofi-emoji";

  buildInputs = with pkgs; [
    # Build tools
    gnumake
    autoconf
    automake
    libtool
    pkg-config
    clang

    # Dependencies
    rofi
    glib
    cairo

    # Clipboard adapters
    wl-clipboard
    wtype
    xclip
    xdotool
    xsel
  ];
}
