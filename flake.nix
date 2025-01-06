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
            meta.mainProgram = "calias";
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
        devShells.default = pkgs.mkShell {
          buildInputs = [ packages.calias ];
          shellHook = ''
            export ROOT_DIR="$PWD"

            ${pkgs.lib.getExe packages.makeAll}
            export CALIAS="$ROOT_DIR/build/calias"
            export ALTLIB="$ROOT_DIR/build/altlib_ext"
          '';
        };
      }
    );
}
