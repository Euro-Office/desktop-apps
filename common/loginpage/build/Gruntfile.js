module.exports = function(grunt) {
    var _ = require('lodash'),
        defaultConfig,
        packageFile;


    grunt.loadNpmTasks('grunt-contrib-clean');
    grunt.loadNpmTasks('grunt-contrib-copy');
    grunt.loadNpmTasks('grunt-contrib-less');
    grunt.loadNpmTasks('grunt-contrib-concat');
    grunt.loadNpmTasks('grunt-contrib-htmlmin');
    grunt.loadNpmTasks('grunt-text-replace');
    grunt.loadNpmTasks('grunt-inline');
    grunt.loadNpmTasks('grunt-terser');
    // grunt.loadNpmTasks('grunt-svgmin');

    function doRegisterInitializeAppTask(name, appName, configFile) {
        return grunt.registerTask('init-build-' + name, 'Initialize build ' + appName, function(){
            defaultConfig = configFile;
            packageFile = require('./' + defaultConfig);

            if (packageFile)
                grunt.log.ok(appName + ' config loaded successfully'.green);
            else
                grunt.log.error().writeln('Could not load config file'.red);
        });
    }

    grunt.initConfig({
        jshint: {
            options: {
                curly: true,
                eqeqeq: true,
                eqnull: true,
                browser: true,
                globals: {
                    jQuery: true
                },
                force: true
            },
            common: ['../src/*.js']
        }
    });

    grunt.registerTask('desktop-app-extra', function() {
        grunt.initConfig({
            pkg: grunt.file.readJSON(defaultConfig),

            less: {
                production: {
                    options: {
                        compress: true,
                        ieCompat: false
                    },
                    files: packageFile.desktop.less.files
                }
            },

            copy: {
                main: {
                    files: packageFile['desktop']['copy']['files']
                }
            },

            concat: {
                options: {
                    stripBanners: true,
                    banner: '/*\n' +
                    ' * Copyright (c) Ascensio System SIA <%= grunt.template.today("yyyy") %>. All rights reserved\n' +
                    ' *\n' +
                    ' * <%= pkg.homepage %> \n' +
                    ' *\n' +
                    ' */\n'
                },
                dist: {
                    files: {
                        "<%= pkg.desktop.concat.files.dest %>" : [packageFile.desktop.concat.files.src]
                    }
                }
            },

            terser: {
                options: {
                    format: {
                        comments: false,
                    },
                    compress: {
                        drop_console: true,
                    },
                },
                langs: {
                    files: {
                        '../deploy/langs.js' : '../locale/*.js'
                    }
                },
                core: {
                    src: '../deploy/build.js',
                    dest: '../deploy/build.min.js',
                },
                dialogconnect: {
                    files: {
                        '../src/dialogconnect.min.js' : ['../src/dialogconnect.js']
                    }
                },
                noconnect: {
                    files: {
                        '../deploy/noconnect.js' : ['../noconnect/code.js', '../noconnect/l10n/*.js']
                    }
                },
            },

            htmlmin: {
                dist: {
                    options: {
                        removeComments: true,
                        collapseWhitespace: true,
                        minifyCSS: true
                    },
                    files: {
                        '../deploy/index.html': '../deploy/index.html'
                    }
                }
            },

            inline: {
                dist: {
                    src: '../deploy/index.html',
                    dest: '../deploy/index.html'
                }
            },

            clean: {
                options: {
                    force: true
                },
                files: packageFile['desktop']['clean']
            }
        });
    });

    grunt.registerTask('connection-error', function() {
        grunt.initConfig({
            terser: {
                options: {
                    format: {
                        comments: false,
                    },
                    compress: {
                        drop_console: true,
                    },
                },
                noconnect: {
                    files: {
                        '../deploy/code.min.js' : ['../noconnect/code.js', '../noconnect/l10n/*.js', '../noconnect/init.js']
                    }
                },
            },

            htmlmin: {
                options: {
                    removeComments: true,
                    collapseWhitespace: true,
                    minifyCSS: true
                },
                dist: {
                    files: {
                        '../deploy/noconnect.html': '../noconnect/index.html.deploy'
                    }
                }
            },

            clean: {
                options: {
                    force: true
                },
                files: ["../deploy/*.svg"]
            },

            copy: {
                main: {
                    files: [{
                        expand: true,
                        cwd: "../noconnect",
                        src: ['*.svg'],
                        dest: '../deploy/'
                    }]
                }
            },

            svgmin: {
                options: {
                    plugins: [
                        {
                            name: 'preset-default',
                            params: {
                                overrides: {
                                    sortAttrs: false,
                                    removeUselessDefs: false,
                                    cleanupIds: false,
                                }
                            }
                        }
                    ]
                },
                dist: {
                    files: [{
                        expand: true,
                        cwd: "../noconnect",
                        src: ['*.svg'],
                        dest: '../deploy',
                    }]
                }
            },
        });
    });

    grunt.registerTask('compile-html', function(task){
        const src_ = [`../deploy/${task == 'noconnect'?'noconnect.html':'index.html'}`];
        grunt.config('replace.insert-css', {
            src: src_,
            overwrite: true,
            replacements: [{
                from: /(\<link[^\<]+stylesheet[^\<]+href="(\w+\.css)\"\>)/g,
                to: function(matchedWord, index, fullText, regexMatches) {
                    var css = grunt.file.read('../deploy/' + regexMatches[1]);
                    return '<style type="text/css">' + css + '</style>';
                }
            },{
                from: /(\<script\s[^\>]+replace[^\>]+"([\w\S\.]+\.js)\"\>\<\/script\>)/g,
                to: function(matchedWord, index, fullText, regexMatches) {
                    if (!grunt.file.exists('../deploy/' + regexMatches[1])) {
                        grunt.log.error().writeln('file does not exists: ' + regexMatches[1]);
                    }

                    var script = grunt.file.read('../deploy/' + regexMatches[1]);
                    return '<script>' + script + '</script>';
                }
            },{
                from: /(\<img\s[^\>]+inline-svg[^\>]+"([\w\S\.]+\.svg)\"\>)/g,
                to: function(matchedWord, index, fullText, regexMatches) {
                    console.log('replace svg ', regexMatches[1]);
                    if (!grunt.file.exists('../deploy/' + regexMatches[1])) {
                        grunt.log.error().writeln('file does not exists: ' + regexMatches[1]);
                    }

                    const svgsource = grunt.file.read('../deploy/' + regexMatches[1]);
                    return svgsource;
                }
            }]
        });

        grunt.config('clean.files', ['../deploy/*.js','../deploy/*.css','../deploy/locale', ...grunt.config('clean.files')]);
        grunt.task.run('replace:insert-css', 'clean');
        // grunt.task.run('replace:insert-css');
    });

    grunt.registerTask('prebuild-svg-sprites', function() {
        require('./sprites/sprites')(grunt, '../');
        grunt.task.run('svg_sprite', 'sprite');
    });

    doRegisterInitializeAppTask('startpage', 'Desktop start page', 'startpage.json');

    grunt.registerTask('deploy-connection-error', ['connection-error', 'terser:noconnect', 'copy', 'htmlmin', /*'svgmin',*/ 'compile-html:noconnect']);
    grunt.registerTask('deploy-desktop-startpage', ['prebuild-svg-sprites', 'desktop-app-extra', 'copy', 'less', 'terser:dialogconnect', 'terser:noconnect',
        'concat', 'clean', 'inline', 'terser:core', 'terser:langs', 'htmlmin', 'compile-html']);
    grunt.registerTask('default', ['init-build-startpage','deploy-desktop-startpage','deploy-connection-error']);
};