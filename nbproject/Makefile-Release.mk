#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/client/Client.o \
	${OBJECTDIR}/client/Conf.o \
	${OBJECTDIR}/client/Detect_linux.o \
	${OBJECTDIR}/utils/EzCurl.o \
	${OBJECTDIR}/utils/json11/json11.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-Wall -Wvla -Wno-deprecated -Werror
CXXFLAGS=-Wall -Wvla -Wno-deprecated -Werror

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lstdc++ -Wl,--no-as-needed -lcurl -pthread

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ./gearsd

./gearsd: ${OBJECTFILES}
	${MKDIR} -p .
	${LINK.cc} -o ./gearsd ${OBJECTFILES} ${LDLIBSOPTIONS} -s

${OBJECTDIR}/client/Client.o: client/Client.cpp 
	${MKDIR} -p ${OBJECTDIR}/client
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -Iprecomp -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/client/Client.o client/Client.cpp

${OBJECTDIR}/client/Conf.o: client/Conf.cpp 
	${MKDIR} -p ${OBJECTDIR}/client
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -Iprecomp -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/client/Conf.o client/Conf.cpp

${OBJECTDIR}/client/Detect_linux.o: client/Detect_linux.cpp 
	${MKDIR} -p ${OBJECTDIR}/client
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -Iprecomp -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/client/Detect_linux.o client/Detect_linux.cpp

${OBJECTDIR}/utils/EzCurl.o: utils/EzCurl.cpp 
	${MKDIR} -p ${OBJECTDIR}/utils
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -Iprecomp -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/utils/EzCurl.o utils/EzCurl.cpp

${OBJECTDIR}/utils/json11/json11.o: utils/json11/json11.cpp 
	${MKDIR} -p ${OBJECTDIR}/utils/json11
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -Iprecomp -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/utils/json11/json11.o utils/json11/json11.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ./gearsd

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
