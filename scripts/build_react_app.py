Import("env")
import os
import shutil
import glob
import json
from re import sub

# Simply run react build script
def createReactAssets( source, target, env ):
    # delete existing folder
    data_dir_path = os.path.join(env.get('PROJECT_DIR'), 'frontend')
    if(os.path.exists(data_dir_path)):
        print('Rename: Deleting the old data directory ' + data_dir_path)
        shutil.rmtree(data_dir_path)
    print('Rename: Creating an empty data directory ' + data_dir_path)
    os.mkdir(data_dir_path)
    # Build the react web application
    react_proj_dir = "react-client"
    os.chdir(react_proj_dir)
    print( '\nReact build: Starting the build process for the React webapp...\n' )
    
    print('\nRunning `npm install`')
    if os.system("npm install") != 0:
        print(f"Error installing dependencies for react application in ./{react_proj_dir}")
        return
    print('\nRunning `npm run build`')
    if os.system("npm run build") != 0:
        print(f"Error building react application in ./{react_proj_dir}")
        return
    os.chdir('..')
env.AddPreAction('$BUILD_DIR/index.html.gz.txt.o', createReactAssets )

# if not os.path.exists(os.path.join(env.get('PROJECT_DIR'), 'frontend', 'index.html.gz')):
#     createReactAssets(None, None, env)
# createReactAssets(None, None, env)