{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    inputs:
    inputs.flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = inputs.nixpkgs.legacyPackages.${system};
        packages = {
          calias = pkgs.stdenv.mkDerivation {
            name = "calias";
            src = ./.;
            buildInputs = [ pkgs.nasm ];
            buildPhase = ''
              make
            '';
            installPhase = ''
              mkdir -p $out/bin
              cp build/calias $out/bin/calias
            '';
          };

          makeAll = pkgs.writeShellApplication {
            name = "makeAll";
            text = ''
              make compiler
              make altlib
              make test
              make perftest
            '';
            runtimeInputs = [
              pkgs.nasm
              pkgs.cmake
            ];
          };
        };
      in
      {
        inherit packages;
        devShells.default = pkgs.mkShell { buildInputs = [ packages.calias ]; };
      }
    );
}
