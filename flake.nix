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
        fs = pkgs.lib.fileset;
        packages = {
          calias = pkgs.stdenv.mkDerivation {
            name = "calias";
            src = fs.toSource {
              root = ./.;
              fileset = fs.unions [
                ./altlib
                ./arch
                ./compiler
                ./docs
                ./stdlib
                ./test
                ./Makefile
              ];
            };
            buildInputs = [ pkgs.nasm ];
            buildPhase = ''
              make compiler
              make altlib
            '';
            checkPhase = ''
              make test
              make perftest
            '';
            installPhase = ''
              mkdir -p $out/bin
              cp -r build $out
              cp build/calias $out/bin/calias
            '';
            meta.mainProgram = "calias";
          };
        };
      in
      {
        inherit packages;
        devShells.default = pkgs.mkShell {
          buildInputs = [ packages.calias ];
          shellHook = ''
            export CALIAS="${pkgs.lib.getExe packages.calias}"
            export ALTLIB="${packages.calias}/build/altlib_ext"
          '';
        };
      }
    );
}
