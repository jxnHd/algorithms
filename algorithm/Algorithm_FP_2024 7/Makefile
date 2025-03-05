# 編譯器和編譯選項
CXX = g++
CXXFLAGS = -std=c++11 -O3

# 源文件和目標文件目錄
SRCDIR = src
OBJDIR = obj

# 定義源文件和目標文件
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))

# 最終執行檔案
TARGET = ap

# 編譯規則
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean
