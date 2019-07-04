git clone https://github.com/glfw/glfw.git external/glfw/
git clone https://github.com/g-truc/glm external/glm/
git clone https://github.com/zeux/meshoptimizer.git external/meshoptimizer/
git clone https://github.com/nothings/stb.git external/stb/
git clone https://github.com/syoyo/tinyobjloader.git external/tinyobjloader/
git clone https://github.com/zeux/volk.git external/volk/
Remove-Item -Path "./external/EASTL/" -Recurse -Force
git clone https://github.com/electronicarts/EASTL.git external/EASTL/
rm external/EASTL/test/packages/EAAssert
rm external/EASTL/test/packages/EABase
rm external/EASTL/test/packages/EAMain
rm external/EASTL/test/packages/EAStdC
rm external/EASTL/test/packages/EATest
rm external/EASTL/test/packages/EAThread
git clone https://github.com/electronicarts/EAAssert.git external/EASTL/test/packages/EAAssert
git clone https://github.com/electronicarts/EABase.git external/EASTL/test/packages/EABase
git clone https://github.com/electronicarts/EAMain.git external/EASTL/test/packages/EAMain
git clone https://github.com/electronicarts/EAStdC.git external/EASTL/test/packages/EAStdC
git clone https://github.com/electronicarts/EATest.git external/EASTL/test/packages/EATest
git clone https://github.com/electronicarts/EAThread.git external/EASTL/test/packages/EAThread

PAUSE