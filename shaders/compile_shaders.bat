cd %1
for %%f in (*.vert *.frag) do C:\VulkanSDK\1.3.250.0\Bin\glslangValidator.exe -V -o %%f.spv %%f
pause