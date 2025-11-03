CXX			=	clang++ -x objective-c++
CC			=	clang
NAME		=	Padentvo
SRCS		=	*.cpp *.m *.mm *.c
INCLUDES	=	*.h *.hpp
SHADERS		=	*.metal
APP_NAME	=	$(NAME).app
APP_CONTENTS	=	$(APP_NAME)/Contents
APP_MACOS	=	$(APP_CONTENTS)/MacOS
APP_RESOURCES	=	$(APP_CONTENTS)/Resources
CPP_FILES	=	$(shell find $(SRCS) -name '*.cpp')
C_FILES		=	$(shell find $(SRCS) -name '*.c')
MM_FILES	=	$(shell find $(SRCS) -name '*.mm')
M_FILES		=	$(shell find $(SRCS) -name '*.m')
HEADERS_H	=	$(shell find $(INCLUDES) -name '*.h') /usr/local/include
HEADERS_HPP	=	$(shell find $(INCLUDES) -name '*.hpp')
METAL_FILES	=	$(shell find $(SHADERS) -name '*.metal')
METAL_AIR	=	$(patsubst $(SHADERS)/%.metal,$(OBJS_DIR)/%.air,$(METAL_FILES))
METAL_METALLIB	=	$(patsubst $(SHADERS)/%.metal,$(OBJS_DIR)/%.metallib,$(METAL_FILES))
METAL_FLAGS	=	#-std=macos-metal2.3
CFLAGS		=	-I./includes -Wall -Wextra -Werror
OBJS_DIR	=	Product
DEPS_DIR	=	$(OBJS_DIR)
PLIST		=	macOSInfo.plist
ICON		=	AppIcon.icns
FLAGS		=	-std=c++20 -ObjC++ -g -I./includes -I./Shaders -I./Frameworks/metal-cpp -I./Frameworks/metal-cpp-extensions -ferror-limit=100 -fobjc-weak -Warc-bridge-casts-disallowed-in-nonarc -Wobjc-missing-super-calls -Wincomplete-implementation #-Wall -Wextra -Werror -fobjc-arc
#LINKERFLAGS	=	-Xlinker -sectcreate -Xlinker __TEXT -Xlinker __info_plist -Xlinker $(PLIST)

CPP_OBJS	=	$(patsubst $(SRCS)/%.cpp,$(OBJS_DIR)/%.o,$(CPP_FILES))
C_OBJS		=	$(patsubst $(SRCS)/%.c,$(OBJS_DIR)/%.o,$(C_FILES))
MM_OBJS		=	$(patsubst $(SRCS)/%.mm,$(OBJS_DIR)/%.o,$(MM_FILES))
M_OBJS		=	$(patsubst $(SRCS)/%.m,$(OBJS_DIR)/%.o,$(M_FILES))
$(shell mkdir -p $(sort $(dir $(C_OBJS))) $(sort $(dir $(CPP_OBJS))) $(sort $(dir $(MM_OBJS))) $(sort $(dir $(M_OBJS))) $(sort $(dir $(METAL_AIR))) $(sort $(dir $(METAL_METALLIB))))
C_DEPS		=	$(patsubst $(OBJS_DIR)/%.o,$(DEPS_DIR)/%.d,$(C_OBJS))
CPP_DEPS	=	$(patsubst $(OBJS_DIR)/%.o,$(DEPS_DIR)/%.d,$(CPP_OBJS))
MM_DEPS		=	$(patsubst $(OBJS_DIR)/%.o,$(DEPS_DIR)/%.d,$(MM_OBJS))
M_DEPS		=	$(patsubst $(OBJS_DIR)/%.o,$(DEPS_DIR)/%.d,$(M_OBJS))
DEP_FILES	=	$(CPP_DEPS) $(C_DEPS) $(MM_DEPS) $(M_DEPS)
$(shell mkdir -p $(sort $(dir $(DEP_FILES))))

_GREEN		=	\033[92;5;118m
_YELLOW		=	\033[93;5;226m
RED			=	\033[31m
GREEN		=	\033[32m
YELLOW		=	\033[33m
CYAN		=	\033[96m
RESET		=	\033[0m
CURSIVE     =	\033[33;3m
GRAY        =	\033[33;2;37m

BOLDBLACK	=	\033[1;30m
BOLDRED		=	\033[1;31m
BOLDGREEN	=	\033[1;32m
BOLDYELLOW	=	\033[1;33m
BOLDBLUE	=	\033[1;34m
BOLDPURPLE	=	\033[1;35m
BOLDCYAN	=	\033[1;36m
BOLDWHITE	=	\033[1;37m

HIBLACK		=	\033[0;90m
HIRED		=	\033[0;91m
HIGREEN		=	\033[0;92m
HIYELLOW	=	\033[0;93m
HIBLUE		=	\033[0;94m
HIPURPLE	=	\033[0;95m
HICYAN		=	\033[0;96m
HIWHITE		=	\033[0;97m

UBLACK		=	\033[4;30m
URED		=	\033[4;31m
UGREEN		=	\033[4;32m
UYELLOW		=	\033[4;33m
UBLUE		=	\033[4;34m
UPURPLE		=	\033[4;35m
UCYAN		=	\033[4;36m
UWHITE		=	\033[4;37m

BBLACK		=	\033[40m
BRED		=	\033[41m
BGREEN		=	\033[42m
BYELLOW		=	\033[43m
BBLUE		=	\033[44m
BPURPLE		=	\033[45m
BCYAN		=	\033[46m
BWHITE		=	\033[47m

SOULIGNAGE	=	\033[4'm

VPATH=./metal-cpp
vpath %.cpp $(SRCS)
vpath %.c $(SRCS)
vpath %.mm $(SRCS)
vpath %.m $(SRCS)

FRAMEWORKS	=	Metal MetalKit QuartzCore AppKit Foundation GameController Cocoa CoreHaptics CoreMotion PHASE AVFAudio MetalFX CoreText CoreGraphics CoreVideo ModelIO CloudKit
LDFLAGS		=	$(addprefix -framework , $(FRAMEWORKS))
MACOS_SDK	=	$(shell xcrun --sdk macosx --show-sdk-path)
CXXFLAGS	+=	-isysroot $(MACOS_SDK)
CFLAGS		+=	-isysroot $(MACOS_SDK)

$(OBJS_DIR)/%.air: %.metal
	@echo "\t$(_YELLOW) compiling metal... $*.metal$(RESET)"
	@mkdir -p $(dir $@)
	xcrun -sdk macosx metal $(METAL_FLAGS) -c $< -o $@

$(OBJS_DIR)/%.metallib: $(OBJS_DIR)/%.air
	@echo "\t$(_YELLOW) creating metallib... $*.metallib$(RESET)"
	xcrun -sdk macosx metallib $< -o $@

$(OBJS_DIR)/%.o: %.c
	@ echo "\t$(_YELLOW) compiling... $*.c$(RESET)"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_DIR)/%.o: %.cpp
	@ echo "\t$(_YELLOW) compiling... $*.cpp$(RESET)"
	$(CXX) $(FLAGS) -c $< -o $@

$(OBJS_DIR)/%.o: %.mm #$(SRCS_DIR)/%.mm $(HEADERS_H)
	@ echo "\t$(_YELLOW) compiling... $*.mm$(RESET)"
	$(CXX) $(FLAGS) -c $< -o $@

$(OBJS_DIR)/%.o: %.m
	@ echo "\t$(_YELLOW) compiling... $*.m$(RESET)"
	$(CC) $(FLAGS) -c $< -o $@

$(DEPS_DIR)/%.d: $(SRCS_DIR)/%.cpp | $(DEPS_DIR)
	@ echo "\t$(_YELLOW) compiling... cpp $*.d$(RESET)"
	$(CXX) $(FLAGS) -MM $< -MT $(@:.d=.o) -MF $@

$(DEPS_DIR)/%.d: $(SRCS_DIR)/%.c | $(DEPS_DIR)
	@ echo "\t$(_YELLOW) compiling... $*.d$(RESET)"
	$(CC) $(FLAGS) -MM $< -MT $(@:.d=.o) -MF $@

$(DEPS_DIR)/%.d: $(SRCS_DIR)/%.mm | $(DEPS_DIR)
	@ echo "\t$(_YELLOW) compiling... $*.d$(RESET)"
	$(CXX) $(FLAGS) -MM $< -MT $(@:.d=.o) -MF $@

$(DEPS_DIR)/%.d: $(SRCS_DIR)/%.m | $(DEPS_DIR)
	@ echo "\t$(_YELLOW) compiling... $*.d$(RESET)"
	$(CC) $(FLAGS) -MM $< -MT $(@:.d=.o) -MF $@

all: init	$(APP_NAME)

init:
	@ if test -f $(NAME); \
		then echo "$(CYAN)\t[program already created]$(RESET)";	\
		else \
		echo "\t$(BPURPLE)[Initialize program]$(RESET)"; \
		$(shell mkdir -p $(sort $(dir $(CPP_OBJS))) $(sort $(dir $(C_OBJS))) $(sort $(dir $(MM_OBJS))))	\
	fi

$(APP_NAME): $(NAME) #$(ICON)
	@echo "\t$(CYAN)[Creating application bundle]$(RESET)"
	@mkdir -p $(APP_MACOS) $(APP_RESOURCES)
	@cp $(NAME) $(APP_MACOS)/
	@mkdir -p $(APP_CONTENTS)
	@cp $(PLIST) $(APP_CONTENTS)/Info.plist
	@if [ -f $(ICON) ]; then cp $(ICON) $(APP_RESOURCES)/AppIcon.icns; fi
	@for metallib in $(METAL_METALLIB); do \
		if [ -f $$metallib ]; then \
			cp $$metallib $(APP_RESOURCES)/; \
			echo "\t$(HIWHITE)Metallib copied: $$metallib$(RESET)"; \
		fi \
	done
	@echo "\t$(SOULIGNAGE)Application bundle created: $(APP_NAME)$(RESET)"

$(NAME):	$(C_OBJS) $(CPP_OBJS) $(MM_OBJS) $(M_OBJS) $(METAL_METALLIB)
	@echo "\t$(CYAN)[Creating program]$(RESET)"
	@echo "\t$(YELLOW) compiling$(_YELLOW)... $(RESET)$(YELLOW)$*.c++$*.cpp$*.mm$*.m$(RESET)"
	$(CXX) $(FLAGS) $(C_OBJS) $(CPP_OBJS) $(MM_OBJS) $(M_OBJS) -o $(NAME) $(LDFLAGS) $(LINKERFLAGS)
	@echo "$(CURSIVE)[Executable created & ready]$(RESET)\n\033[32mCompiled! ᕦ(\033[31m♥\033[32m_\033[31m♥\033[32m)ᕤ"
	@echo "$(_GREEN)object files were in test$(RESET)"

clean:
	@echo "$(GRAY)[cleaning up .out & objects files]$(RESET)"
	rm -rf $(OBJS_DIR) Shaders/*.air Shaders/*.metallib $(APP_NAME)

fclean:	clean
	@printf "$(RED)[cleaning up .out, objects & library files]$(_NC)\n$(URED)Deleting EVERYTHING! ⌐(ಠ۾ಠ)¬$(RESET)$(BOLDRED)$(RESET)\n"
	rm -f $(NAME)

re:		fclean all

.PHONY:	all clean fclean re
