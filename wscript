
#
# This file is the default set of rules to compile a Pebble project.
#
# Feel free to customize this to your needs.
#

import os.path
import json

# Get the version from appinfo.json
json_data=open('appinfo.json')

data = json.load(json_data)
json_data.close()

versionLabel = data["versionLabel"]
versionVal = versionLabel.replace('.', '')
versionDef = "-DVERSION=" + versionVal
versionTxtDef = "-DVERSION_TXT=\"" + versionLabel + "\""

print(versionDef)
print(versionTxtDef)

top = '.'
out = 'build'

def options(ctx):
    ctx.load('pebble_sdk')

def configure(ctx):
    ctx.load('pebble_sdk')

def build(ctx):
    ctx.load('pebble_sdk')
    
    ctx.env.CFLAGS += [versionDef]
    ctx.env.CFLAGS += [versionTxtDef]
    
    #ctx.env.CFLAGS += ["-fstack-usage"]
    #ctx.env.CFLAGS += ["-Os"]

    ctx.pbl_program(source=ctx.path.ant_glob('src/**/*.c'),
                    target='pebble-app.elf')
                   
    # Concatenate all JS files into pebble-js-app.js prior to building.
    all_js = "\n".join([node.read() for node in ctx.path.ant_glob('src/js/**/*.js', excl='src/js/build/pebble-js-app.max.js src/js/build/pebble-js-app.js')])
    out_js_node = ctx.path.make_node('src/js/build/pebble-js-app.max.js')
    out_js_node.write(all_js)
    build_js = ctx.path.make_node('src/js/build/pebble-js-app.js')
    
    # Validate the javascript    
    #ctx(rule='(java -jar ../../jslint/jslint4java-2.0.4.jar --sloppy --white --vars ${SRC})',
    #source=out_js_node, target=build_js)
    
    # Minify
    ctx(rule='(java -jar ../../yui/yuicompressor-2.4.8.jar -o ${TGT} ${SRC})',
    source=out_js_node, target=build_js)

    if os.path.exists('worker_src'):
        ctx.pbl_worker(source=ctx.path.ant_glob('worker_src/**/*.c'),
                        target='pebble-worker.elf')
        ctx.pbl_bundle(elf='pebble-app.elf',
                        worker_elf='pebble-worker.elf',
                        js=build_js)
    else:
        ctx.pbl_bundle(elf='pebble-app.elf',
                        js=build_js)
