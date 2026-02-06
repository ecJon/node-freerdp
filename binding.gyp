{
    "targets": [
        {
            "target_name": "node-freerdp",
            "sources": [
              "context.cc",
              "generator.cc",
              "rdp.cc",
              "bridge.cc",
              "node-freerdp.cc"
            ],
            "include_dirs": [
                "<!(node -e \"require('nan')\")",
                "/Users/ecjon/freerdp-static/include/freerdp3",
                "/Users/ecjon/freerdp-static/include/winpr3"
            ],
            "libraries": [
                "/Users/ecjon/freerdp-static/lib/libfreerdp-client3.a",
                "/Users/ecjon/freerdp-static/lib/libfreerdp3.a",
                "/Users/ecjon/freerdp-static/lib/libwinpr3.a"
            ],
            "cflags+": ["-isysroot", "/Library/Developer/CommandLineTools/SDKs/MacOSX15.5.sdk"],
            "cflags_cc+": ["-isysroot", "/Library/Developer/CommandLineTools/SDKs/MacOSX15.5.sdk"],
            "ldflags+": ["-isysroot", "/Library/Developer/CommandLineTools/SDKs/MacOSX15.5.sdk"],
            "conditions": [
                ["OS=='mac'", {
                    "libraries": [
                        "-framework Security",
                        "-framework CoreFoundation",
                        "-framework CoreServices",
                        "-framework CoreGraphics",
                        "-framework CoreText",
                        "-framework ApplicationServices",
                        "-framework Carbon",
                        "-framework IOKit",
                        "/opt/homebrew/opt/openssl@3/lib/libssl.a",
                        "/opt/homebrew/opt/openssl@3/lib/libcrypto.a",
                        "-lz",
                        "-lpthread",
                        "-lm"
                    ],
                    "xcode_settings": {
                        "OTHER_LDFLAGS": [
                            "-framework Security",
                            "-framework CoreFoundation",
                            "-framework CoreServices",
                            "-framework CoreGraphics",
                            "-framework CoreText",
                            "-framework ApplicationServices",
                            "-framework Carbon",
                            "-framework IOKit"
                        ]
                    }
                }],
                ["OS=='linux'", {
                    "libraries": [
                        "-lssl",
                        "-lcrypto",
                        "-lz",
                        "-lpthread",
                        "-lm",
                        "-lrt"
                    ]
                }],
                ["OS=='win'", {
                    "libraries": [
                        "libssl.lib",
                        "libcrypto.lib",
                        "crypt32.lib",
                        "secur32.lib"
                    ],
                    "msvs_settings": {
                        "VCLinkerTool": {
                            "AdditionalLibraryDirectories": [
                                "$(SolutionDir)libs\\win32"
                            ]
                        }
                    }
                }]
            ]
        }
    ]
}
