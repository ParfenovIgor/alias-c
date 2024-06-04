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
        packages.calias = pkgs.stdenv.mkDerivation {
          name = "calias";
          src = ./.;
          buildInputs = [pkgs.nasm];
          installPhase = ''
            mkdir -p $out/bin
            cp build/calias $out/bin/calias
          '';
        };
      in
      {
        inherit packages;
        devShells = pkgs.mkShell {
          buildInputs = [ packages.default ];
          bash.extra = '''';
        };
      }
    );
}
