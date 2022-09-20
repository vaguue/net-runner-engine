{
  'variables': {
    'openssl_fips': '',
    'includeDir': '<!(node -p \"require(\'./install/ns3\').includeDir\")',
    'libDir': '<!(node -p \"require(\'./install/ns3\').libDir\")',
    'boostDir': '<!(node -p \"require(\'./install/ns3\').boostDir\")',
    'rpath': '<!(node -p \"require(\'./install/ns3\').rpath\")',
  },
  'targets': [
    {
      'target_name': 'wrapper-native',
      'sources': ['src/wrapper.cc'],
      'include_dirs': ['<!@(node -p \"require(\'node-addon-api\').include\")', '<(boostDir)','<(includeDir)', '<(includeDir)/ns3-dev', '<(includeDir)/openflow'],
      'dependencies': ['<!(node -p \"require(\'node-addon-api\').gyp\")'],
      'cflags!': ['-fno-exceptions'],
      'cflags_cc!': ['-fno-exceptions', '-fno-rtti'],
      'cflags_cc': ['--std=c++17', '-lstdc++fs', '-I<(includeDir)', '-L<(libDir)'],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.7',
        'OTHER_LDFLAGS': [
          '-Wl,-rpath,\'<(rpath)\'',
        ],
      },
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 }
      }, 
      'link_settings': {
        'library_dirs': ['<(libDir)'],
        'libraries': ['<!@(node -p \"require(\'./install/ns3\').libs\")'], 
        'ldflags': [
          '-Wl,-s -Wl,--disable-new-dtags -Wl,-rpath=\'<(rpath)\'',
        ]
      }
    }
  ], 
}
