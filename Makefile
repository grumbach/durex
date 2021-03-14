
##################################### BIN ######################################

NAME = durex

# Order matters, "payload.c" must be last !
SRC = main.c            \
      errors.c          \
      utils.c           \
      setup_payload.c   \
      payload.c

CC = clang
SRCDIR = srcs
OBJDIR = objs
OBJ = $(addprefix ${OBJDIR}/, $(SRC:.c=.o))
DEP = $(addprefix ${OBJDIR}/, $(SRC:.c=.d))

override CFLAGS += -Wall -Wextra -pedantic
LDFLAGS = -Iincludes/ -fsanitize=address,undefined

#################################### COLORS ####################################

BY = "\033[33;1m"
BR = "\033[31;1m"
BG = "\033[32;1m"
BB = "\033[34;1m"
BM = "\033[35;1m"
BC = "\033[36;1m"
BW = "\033[37;1m"
Y = "\033[33m"
R = "\033[31m"
G = "\033[32m"
B = "\033[34m"
M = "\033[35m"
C = "\033[36m"
W = "\033[0m""\033[32;1m"
WR = "\033[0m""\033[31;5m"
WY = "\033[0m""\033[33;5m"
X = "\033[0m"
UP = "\033[A"
CUT = "\033[K"

#################################### RULES #####################################

all: art ${NAME}

${NAME}: ${OBJ}
	@echo ${B}Compiling [${NAME}]...${X}
	@${CC} ${LDFLAGS} -o $@ ${OBJ}
	@echo ${G}Success"   "[${NAME}]${X}

${OBJDIR}/%.o: ${SRCDIR}/%.c
	@echo ${Y}Compiling [$@]...${X}
	@mkdir -p ${OBJDIR}
	@${CC} ${CFLAGS} ${LDFLAGS} -c -o $@ $<
	@printf ${UP}${CUT}

#################################### DEBUG #####################################

debug: fclean
	${MAKE} all CFLAGS:="-DDEBUG -g"

check:
	service ${NAME} status

nc:
	nc localhost 4242

reset:
	-sudo killall ${NAME}
	-sudo systemctl stop ${NAME}
	-sudo systemctl disable ${NAME}
	-sudo rm -f /etc/systemd/system/${NAME}
	-sudo rm -f /etc/systemd/system/${NAME}.service
	-sudo rm -f /usr/lib/systemd/system/${NAME}
	-sudo rm -f /usr/lib/systemd/system/${NAME}.service
	-sudo systemctl daemon-reload
	-sudo systemctl reset-failed
	-sudo rm -f /usr/sbin/${NAME}

################################### GENERAL ####################################

clean:
	@echo ${R}Cleaning"  "[objs]...${X}
	@/bin/rm -Rf ${OBJDIR}

fclean: clean
	@echo ${R}Cleaning"  "[${NAME}]...${X}
	@/bin/rm -f ${NAME}
	@/bin/rm -Rf ${NAME}.dSYM

re: fclean all

################################## DECORATION ##################################

art:
	@echo ${BB}
	@echo "                                                  _  _"
	@echo "                                                 (\\\\\\( \\"
	@echo "                                                  \`.\\-.)"
	@echo "                              _...._            _,-'   \`-."
	@echo "\\                           ,'      \`-._.---.,-'       "${WR}"*"${X}${BB}"  \\"
	@echo " \\\`.                      ,'                               \`."
	@echo "  \\ \`-...__              /                           .   .:  y"
	@echo "   \`._     \`\`--..__     /                           ,'\`---._/"
	@echo "      \`-._         \`\`--'                      |    /_"
	@echo "          \`.._                   _            ;   <_ \\"
	@echo "              \`--.___             \`.           \`-._ \\ \\"
	@echo "                     \`--<           \`.     (\\ _/)/ \`.\\/"
	@echo "                         \\            \\     ( "${WR}"**"${X}${BB}"\\  /_/"
	@echo "                          \`.           ;      \`._y"
	@echo "                            \`--.      /    _../"
	@echo "                                \\    /__..'"
	@echo "                                 ;  //"
	@echo "                                <   \\\\\\"
	@echo "                                 \`.  \\\\\\"
	@echo "                                   \`. \\\\\\_ __"
	@echo "                                     \`.\`-'  \\\\\\"
	@echo "                                       \`----''"
	@echo ${X}

.PHONY: all clean fclean re art

-include ${DEP}
