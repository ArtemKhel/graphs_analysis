{
  inputs = {
    nixpkgs = {
      type = "indirect";
      id = "nixpkgs";
    };
    flake-parts.url = "github:hercules-ci/flake-parts";
  };

  outputs = inputs @ {
    self,
    nixpkgs,
    ...
  }:
    inputs.flake-parts.lib.mkFlake {inherit inputs;} {
      systems = ["x86_64-linux"];

      perSystem = {
        system,
        pkgs,
        ...
      }: {
        devShells.default = let
          deps = with pkgs; [
            stdenv.cc.cc.lib
            python3
            ruff
            uv
            suitesparse-graphblas
          ];
        in
          pkgs.mkShell {
            LD_LIBRARY_PATH = "${pkgs.stdenv.cc.cc.lib}/lib";
            buildInputs = deps;
            shellHook = ''
              source ./.venv/bin/activate
            '';
          };
      };

    };
}
