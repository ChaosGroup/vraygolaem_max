To compile VRayGolaem plugin:
=============================

*) Open the .vcproj file in Visual Studio and let it convert if needed;
*) Adjust paths to the V-Ray SDK and the 3ds Max SDK and the output file name as needed;
*) For 3ds Max 2013 and later, make sure the character set is set to "Unicode".

To use VRayGolaem:
==================

*) Copy the vraygolaemNNNN.dlo file into the "[3dsMaxRoot]\plugins\vrayplugins" folder (or whereever the V-Ray plugins for 3ds Max are)

*) Copy the vray_glmCrowdVRayPlugin.dll plugin for V-Ray 3.0 Standalone to the folder with the rest of the V-Ray plugins (f.e. "C:\Program Files\Chaos Group\V-Ray\RT for 3ds Max 2014 for x64\bin\plugins") OR add the path to them to the VRAY30_RT_FOR_3DSMAX2014_PLUGINS environment variable so that the plugin can be found and loaded.

*) Set the PATH environment variable to also contain the path to the Golaem "bin" folder so that the various Golaem DLLs can be found.

*) Make sure that the environment variables needed for V-Ray RT are set up correctly (i.e. VRAY30_RT_FOR_3DSMAX2014_PLUGINS etc).

*) The VRayGolaem plugin in 3ds Max can be created from the "Create" tab of the Command planel, "VRay" category. Click on the VRayGolaem button, then click and drag in a viewport to create it. Then click on the "cache_file" button to specify a .vrscene file containing the GolaemCrowd plugin. The .vrscene file MUST NOT contain any SettingsXXXXX plugins - they will mess up the rendering. Use the "shader_file" button to specify a .vrscene file with the shaders that will be used for rendering. You can control the texture search paths using the VRAY_ASSETS_PATH environment variable if needed.

Official help page: http://golaem.com/content/doc/golaem-crowd-documentation/rendering-v-ray
