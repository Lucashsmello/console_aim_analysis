CINC = -I"$(OPENCV3.4_MINGW_DIR)\include" -I"$(BOOST_DIR)" -fopenmp
LDSEARCHPATH = -L"$(OPENCV3.4_MINGW_DIR)\x64\mingw\bin" -L"$(BOOST_DIR)\stage\lib"
#LDSEARCHPATH = -L$(Path)
LDFLAGS = -lopencv_core340 -lopencv_imgproc340 -lopencv_imgcodecs340 -lopencv_highgui340 -lopencv_videoio340 -lws2_32 -fopenmp
#LDFLAGS := $(shell pkg-config --cflags --libs C:\myPrograms\opencv3.4\mingwBuild\unix-install\opencv.pc) -fopenmp
CFLAGS = -Wall -Wtype-limits -Wno-sign-compare -Wno-unused-variable -Wno-unused-function -Wno-strict-aliasing -std=c++11 -O3 $(CINC)
OBJDIR	= .obj




mainOBJS = main.o gimx_api.o aim_analysis_core.o Acquisition.o utils/ImageUtils.o utils/EventVideoCapture.o aim_analysis/AimData.o
mainOBJS := $(addprefix $(OBJDIR)/,$(mainOBJS))
main: $(OBJDIR) $(mainOBJS)
	$(CXX) -o $@ $(mainOBJS) $(LDSEARCHPATH) $(LDFLAGS) -lm -lboost_thread-mgw72-mt-x64-1_66 -lboost_system-mgw72-mt-x64-1_66
	
main2OBJS = main2.o gimx_api.o aim_analysis_core.o Acquisition.o utils/ImageUtils.o utils/EventVideoCapture.o aim_analysis/AimData.o
main2OBJS := $(addprefix $(OBJDIR)/,$(main2OBJS))
main2: $(OBJDIR) $(main2OBJS)
	$(CXX) -o $@ $(main2OBJS) $(LDSEARCHPATH) $(LDFLAGS) -lm -lboost_thread-mgw72-mt-x64-1_66 -lboost_system-mgw72-mt-x64-1_66	


$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir $(OBJDIR) && cd $(OBJDIR) && mkdir utils && mkdir aim_analysis

clean:
	rd /s $(OBJDIR)

