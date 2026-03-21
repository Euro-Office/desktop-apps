
module.exports = (grunt, rootpathprefix) => {
    const _path = rootpathprefix || '../../';
    const sprite_toolicons_name = 'toolicons';

    const helpers = {
        half: num => {return num/2;},
        scaled: (num, factor) => {return num / factor;}
    };

    const configTemplate = opts => {
        const _res_root = `${_path}res/img`,
            _scaled_path = `${opts.scale}/${opts.extpath ? opts.extpath : '.'}`,
            _icons_dir = `${opts.iconsdir}/${_scaled_path}`;
            _dest_name = (opts.scale != '1x' ? opts.spritename + '@' + opts.scale : opts.spritename) + '.png'
        return {
            src: [`${_res_root}/${_icons_dir}/*.png`],
            dest: `${_res_root}/generated/${_dest_name}`,
            destCss: `${_res_root}/../../src/css/${opts.spritename}@${opts.scale}.less`,
            imgPath: `../../res/img/generated/${_dest_name}`,
            cssTemplate: `${_res_root}/${_icons_dir}/.css.handlebars`,
            algorithm: 'top-down',
            cssHandlebarsHelpers: helpers
        };
    };

    grunt.initConfig({
        svg_sprite:{
            options: {
                svg: {
                    rootAttributes: {
                        xmlns:'http://www.w3.org/2000/svg',
                        fill: 'none',
                    },
                },
                shape: {
                    id: {
                        separator: ""
                    },
                    transform: [],
                    dimension: {
                        attributes: true
                    }
                },
                mode: {
                    symbol: {
                    },
                },
            },
            connect: {
                src: [`${_path}res/img/connect*.svg`],
                dest: `${_path}res/img/`,
                options: {
                    mode: {
                        symbol: {
                            inline: true,
                            dest: './generated',
                            sprite: `connect.svg`,
                        },
                    },
                },
            },
            welcome: {
                src: [`${_path}res/img/welcome*.svg`],
                dest: `${_path}res/img/`,
                options: {
                    mode: {
                        symbol: {
                            inline: true,
                            dest: './generated',
                            sprite: `welcome.svg`,
                        },
                    },
                }
            },
            formats: {
                src: [`${_path}res/img/formats-svg/*.svg`],
                dest: `${_path}res/img/`,
                options: {
                    mode: {
                        symbol: {
                            inline: true,
                            dest: './generated',
                            sprite: `formats.svg`,
                        },
                    },
                }
            },
            common: {
                src: [`${_path}res/img/common-svg/*.svg`],
                dest: `${_path}res/img/`,
                options: {
                    mode: {
                        symbol: {
                            inline: true,
                            dest: './generated',
                            sprite: `common.svg`,
                        },
                    },
                }
            },
            update_status: {
                src: [`${_path}res/img/update_status/*.svg`],
                dest: `${_path}res/img/`,
                options: {
                    mode: {
                        symbol: {
                            inline: true,
                            dest: './generated',
                            sprite: `update_status.svg`,
                        },
                    },
                }
            },
            logo: {
                src: [`${_path}res/img/idx-logo*.svg`],
                dest: `${_path}res/img/`,
                options: {
                    mode: {
                        symbol: {
                            inline: true,
                            dest: './generated',
                            sprite: `logo.svg`,
                        },
                    },
                }
            },
            tpltype: {
                src: [`${_path}res/img/template-type/*.svg`],
                dest: `${_path}res/img/`,
                options: {
                    mode: {
                        symbol: {
                            inline: true,
                            dest: './generated',
                            sprite: `templatetype.svg`,
                        },
                    },
                }
            },
            toolicons: {
                src: [`${_path}res/img/toolicons/1x/*.svg`],
                dest: `${_path}res/img/`,
                options: {
                    mode: {
                        symbol: {
                            inline: true,
                            dest: './generated',
                            sprite: `toolicons.svg`,
                        },
                    },
                }
            },
            toolicons20: {
                src: [`${_path}res/img/toolicons20/1x/*.svg`],
                dest: `${_path}res/img/`,
                options: {
                    mode: {
                        symbol: {
                            inline: true,
                            dest: './generated',
                            sprite: `toolicons20.svg`,
                        },
                    },
                }
            },
        },
        sprite: {
            'toolicon1.5x': configTemplate({
                iconsdir: sprite_toolicons_name,
                spritename: sprite_toolicons_name,
                scale: '1.5x'
            }),
            'toolicon1.25x': configTemplate({
                iconsdir: sprite_toolicons_name,
                spritename: sprite_toolicons_name,
                scale: '1.25x'
            }),
            'toolicon1.75x': configTemplate({
                iconsdir: sprite_toolicons_name,
                spritename: sprite_toolicons_name,
                scale: '1.75x'
            }),
            'toolicon20_1.5x': configTemplate({
                iconsdir: `${sprite_toolicons_name}20`,
                spritename: `${sprite_toolicons_name}20`,
                scale: '1.5x'
            }),
            'toolicon20_1.25x': configTemplate({
                iconsdir: `${sprite_toolicons_name}20`,
                spritename: `${sprite_toolicons_name}20`,
                scale: '1.25x'
            }),
            'toolicon20_1.75x': configTemplate({
                iconsdir: `${sprite_toolicons_name}20`,
                spritename: `${sprite_toolicons_name}20`,
                scale: '1.75x'
            }),
        },
        replace_allconnect: {                   //when fill =#fff the fill turns orange on the light theme page
            dist: {
                files:[ {
                    src: [`${_path}res/img/allconnect.svg`],
                    dest: `${_path}res/img/`,
                }],
                options: {
                    replacements: [{
                        pattern: 'fill="#fff"',
                        replacement: 'fill="white"',
                    }]
                }
            }
        }
    });

    grunt.loadNpmTasks('grunt-svg-sprite');
    grunt.loadNpmTasks('grunt-spritesmith');
    grunt.registerTask('generate-sprite', ['svg_sprite', 'replace_allconnect']);
}