CINC = -I"C:\myPrograms\opencv3.4\mingwBuild2\install\include" -I"$(BOOST_DIR)"
LDSEARCHPATH = -L"C:\myPrograms\opencv3.4\mingwBuild2\install\x64\mingw\bin" -L"$(BOOST_DIR)\stage\lib"
#LDSEARCHPATH = -L$(Path)
LDFLAGS = -lopencv_core340 -lopencv_imgproc340 -lopencv_imgcodecs340 -lopencv_highgui340 -lopencv_videoio340
#LDFLAGS := $(shell pkg-config --cflags --libs C:\myPrograms\opencv3.4\mingwBuild\unix-install\opencv.pc) -fopenmp
CFLAGS = -g -Wall -Wno-sign-compare -Wno-unused-variable -Wno-unused-function -Wno-strict-aliasing -std=c++11 -O3 $(CINC)
OBJDIR	= .obj




mainOBJS = main.o gimx_api.o aim_analysis_core.o Acquisition.o utils/ImageUtils.o utils/EventVideoCapture.o
mainOBJS := $(addprefix $(OBJDIR)/,$(mainOBJS))
main: $(OBJDIR) $(mainOBJS)
	$(CXX) -o $@ $(mainOBJS) $(LDSEARCHPATH) $(LDFLAGS) -lm -lboost_thread-mgw72-mt-x64-1_66 -lboost_system-mgw72-mt-x64-1_66


$(OBJDIR)/%.o: %.c
	gcc $(CFLAGS) -c $< -o $@
	
$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir $(OBJDIR) && cd $(OBJDIR) && mkdir utils

clean:
	rd /s $(OBJDIR)

