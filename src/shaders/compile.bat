cd %1
for %%f in (*.vert) do (
    "C:\VulkanSDK\1.3.250.0\Bin\glslc.exe" "%%~nf.vert" -o "%%~nf.vert.spv"
)
for %%f in (*.frag) do (
    "C:\VulkanSDK\1.3.250.0\Bin\glslc.exe" "%%~nf.frag" -o "%%~nf.frag.spv"
)
pause