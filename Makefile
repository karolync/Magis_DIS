# Simplified existing Makefile
# Contact Sanha Cheong, sanha@stanford.edu for questions

################################################################################
# Magis DIS Makefile
################################################################################
PROJECT_ROOT=./

################################################################################
# Key paths and settings
################################################################################
CFLAGS += -std=c++11
ifeq ($(wildcard ${OPT_INC}),)
CXX = g++ ${CFLAGS}
ODIR  = .obj/build
SDIR  = ./src
MKDIR = mkdir -p
PLATFORM = $(shell uname)
ifeq ($(PLATFORM),Darwin)
OS = mac
endif
endif

OUTPUTNAME1 = acquisitionFunc
OUTPUTNAME2 = camCharacterizationDataAcquisition
OUTDIR = ./bin${D}

################################################################################
# Dependencies
################################################################################
# Spinnaker library dependencies
SPINNAKER_LIB = -L/opt/spinnaker/lib -lSpinnaker

################################################################################
# Master inc/lib/obj/dep settings
################################################################################
_OBJ1 = acquisitionFunc.o
OBJ1 = $(patsubst %,$(ODIR)/%,$(_OBJ1))
_OBJ2 = camCharacterizationDataAcquisition.o
OBJ2 = $(patsubst %,$(ODIR)/%,$(_OBJ2))
INC = -I./include
INC += -I/opt/spinnaker/include
LIB += -Wl,-Bdynamic ${SPINNAKER_LIB}

################################################################################
# Rules/recipes
################################################################################
# Final binary
${OUTPUTNAME1}: ${OBJ1}
	${CXX} -o ${OUTPUTNAME1} ${OBJ1} ${LIB}
	mv ${OUTPUTNAME1} ${OUTDIR}

# Intermediate object files
${OBJ1}: ${ODIR}/%.o : ${SDIR}/%.cpp
	@${MKDIR} ${ODIR}
	${CXX} ${CFLAGS} ${INC} -Wall -D LINUX -c $< -o $@

# Clean up intermediate objects
clean_obj:
	rm -f ${OBJ1}
	@echo "intermediate objects cleaned up!"

# Clean up everything.
clean: clean_obj
	rm -f ${OUTDIR}/${OUTPUTNAME1}
	@echo "all cleaned up!"

# Final binary
${OUTPUTNAME2}: ${OBJ2}
	${CXX} -o ${OUTPUTNAME2} ${OBJ2} ${LIB}
	mv ${OUTPUTNAME2} ${OUTDIR}

# Intermediate object files
${OBJ2}: ${ODIR}/%.o : ${SDIR}/%.cpp
	@${MKDIR} ${ODIR}
	${CXX} ${CFLAGS} ${INC} -Wall -D LINUX -c $< -o $@

# Clean up intermediate objects
clean_obj:
	rm -f ${OBJ2}
	@echo "intermediate objects cleaned up!"

# Clean up everything.
clean: clean_obj
	rm -f ${OUTDIR}/${OUTPUTNAME2}
	@echo "all cleaned up!"


