{
    "targets":[
        {			
			"variables": {
				"build_target_name":"ZegoVideoFilter",
				 # x64-是否编译64位，'true'-编译64位
				"x64":"<!(node -e \"console.log(process.env.x64||'false')\")",
                "plugin_version": "<!(node -e \"console.log(process.env.plugin_version||'')\")",
			},
            
            'defines': [
                'PLUGIN_VERSION=<(plugin_version)'
            ],
            
            "target_name":"<(build_target_name)",

            "sources":[
				"src/JSMessageLoop.cpp",
                "src/ZegoVideoFilter.cpp",
                "src/ZGExternalVideoFilterFactory.cpp",
                "src/VideoFilterProcessBase.cpp",
                "src/FuBeautifyFilter.cpp"
				
            ],
            "include_dirs":[
                "<!(node -e \"require('nan')\")",
                '<(module_root_dir)/sdk/include'
            ],
			'conditions': [
				['OS=="win"',{
					"include_dirs":[
						'<(module_root_dir)/sdk/fusdk/Windows/include'
					],
					'conditions': [
						['x64=="false"', {
						  'product_dir': '<(module_root_dir)/electron_libs_out/x86',
						  'library_dirs':[
							'<(module_root_dir)/sdk/fusdk/Windows/win32_release'
						  ],
						  'copies':[{
							'destination':'<(module_root_dir)/electron_libs_out/x86',
							'files':[								
								'<(module_root_dir)/sdk/fusdk/Windows/win32_release/nama.dll',
								'<(module_root_dir)/sdk/fusdk/Windows/win32_release/libsgemm.dll',
							]}
						  ],
						  'msvs_postbuild': r'echo F | xcopy /y "$(SolutionDir)\Release\<(build_target_name).pdb" "$(SolutionDir)\..\electron_libs_pdbs\x86\<(build_target_name).pdb" & del /f /s /q "$(SolutionDir)\..\electron_libs_out\<(build_target_name).exp" & del /f /s /q "$(SolutionDir)\..\electron_libs_out\<(build_target_name).iobj" & del /f /s /q "$(SolutionDir)\..\electron_libs_out\<(build_target_name).ipdb" & del /f /s /q "$(SolutionDir)\..\electron_libs_out\<(build_target_name).lib"',

						}, 'x64=="true"', {
						  'product_dir': '<(module_root_dir)/electron_libs_out/x64',
						  'library_dirs':[
							'<(module_root_dir)/sdk/fusdk/Windows/win64_release'
						  ],
						  'copies':[{
							'destination':'<(module_root_dir)/electron_libs_out/x64',
							'files':[
								'<(module_root_dir)/sdk/fusdk/Windows/win64_release/nama.dll',
								'<(module_root_dir)/sdk/fusdk/Windows/win64_release/libsgemm.dll',
							]}
						  ],
						  'configurations': {
							'Release': {
							  'msvs_target_platform': 'x64',
							},
						  },
						  'msvs_postbuild': r'echo F | xcopy /y "$(SolutionDir)\Release\<(build_target_name).pdb" "$(SolutionDir)\..\electron_libs_pdbs\x64\<(build_target_name).pdb" & del /f /s /q "$(SolutionDir)\..\electron_libs_out\<(build_target_name).exp" & del /f /s /q "$(SolutionDir)\..\electron_libs_out\<(build_target_name).iobj" & del /f /s /q "$(SolutionDir)\..\electron_libs_out\<(build_target_name).ipdb" & del /f /s /q "$(SolutionDir)\..\electron_libs_out\<(build_target_name).lib"',
						}],
					],

					'link_settings':{
						'libraries':[
						'nama.lib'
						] 
					},
                   
					# msvs_settings
					'configurations': {
						'Release': {
							'msvs_settings': {
								'VCCLCompilerTool': {
								# 多线程 MT-0 MTD-1 MD-2 MDD-3
								'RuntimeLibrary': '0',
								# 完全优化 /Os
								'Optimization': '2',
								# 使用内部函数 /Oi
								'EnableIntrinsicFunctions': 'true',
								# 程序数据库 (/Zi)
								'DebugInformationFormat': '3',
								},
								'VCLinkerTool': {
									'GenerateDebugInformation': 'true',
									'GenerateMapFile': 'false',
								},
							}, 
						},
					},  
                  }
				]               
			],			
        }
    ]
}