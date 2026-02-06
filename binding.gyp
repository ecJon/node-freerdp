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
            "libraries": [
              "-L/Users/ecjon/freerdp-install/lib",
              "-lfreerdp-client3",
              "-lfreerdp3",
              "-lwinpr3"
            ],
            "include_dirs" : [
        "<!(node -e \"require('nan')\")",
        "/Users/ecjon/freerdp-install/include/freerdp3",
        "/Users/ecjon/freerdp-install/include/winpr3",
        "/Library/Developer/CommandLineTools/SDKs/MacOSX15.5.sdk/usr/include/c++/v1"
      ],
            "cflags+": ["-isysroot", "/Library/Developer/CommandLineTools/SDKs/MacOSX15.5.sdk"],
            "cflags_cc+": ["-isysroot", "/Library/Developer/CommandLineTools/SDKs/MacOSX15.5.sdk"],
            "ldflags+": ["-isysroot", "/Library/Developer/CommandLineTools/SDKs/MacOSX15.5.sdk"],
            "conditions": [
              ["OS=='mac'", {
                "xcode_settings": {
                  "DYLIB_INSTALL_NAME_BASE": "@rpath",
                  "LD_RUNPATH_SEARCH_PATHS": ["/Users/ecjon/freerdp-install/lib"]
                }
              }]
            ]
        }
    ],
}