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
          # graphblas = pkgs.suitesparse-graphblas.overrideAttrs (oldAttrs: rec {
          #   # version =  "7.4.4";
          #   version =  "10.2.0";
          #   src = pkgs.fetchFromGitHub {
          #     owner = "DrTimothyAldenDavis";
          #     repo = "GraphBLAS";
          #     rev = "v${version}";
          #     hash = "sha256-De2ZDP4Lsik3+Wz6yvm+ro4Q7Q8L5ZDdBaYXHg4LO20=";
          #   };
          # });
          graphblas = pkgs.suitesparse-graphblas;
          deps = with pkgs; [
            stdenv.cc.cc.lib
            python3
            ruff
            uv
            # suitesparse-graphblas
            # suitesparse-graphblas.dev
            # graphblas
            # graphblas.dev
            pkg-config
            cxxopts

    opencl-headers
    ocl-icd
            # spark
          ];
        in
          pkgs.mkShell rec {
            LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath [pkgs.stdenv.cc.cc.lib 
              #graphblas
            ];
            #CPATH = "${graphblas.dev}/include/suitesparse";
            #PKG_CONFIG_PATH = "${graphblas.dev}/lib/pkgconfig";
            SPARK_HOME = "${pkgs.spark}"; 
            buildInputs = deps;
            shellHook = ''
              source ./.venv/bin/activate

              zips=$(printf "%s:" "$SPARK_HOME"/python/lib/*.zip)
              zips=''${zips%:}
              export PYTHONPATH="$zips:''${PYTHONPATH}"
            '';
          };
      };

    };
}
