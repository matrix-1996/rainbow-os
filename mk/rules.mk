# Copyright (c) 2017, Thierry Tremblay
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

.SUFFIXES:
.SUFFIXES: .c .cpp .h. .hpp .s .o

$(BUILDDIR)/%.c.o $(BUILDDIR)/%.c.d: %.c
	@mkdir -p $(dir $@)
	$(CC) $(ARCH_FLAGS) $(CFLAGS) $(CPPFLAGS) -MMD -c $< -o $(@:%.d=%.o)

$(BUILDDIR)/%.cpp.o $(BUILDDIR)/%.cpp.d: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(ARCH_FLAGS) $(CXXFLAGS) $(CPPFLAGS) -MMD -c $< -o $(@:%.d=%.o)

$(BUILDDIR)/%.s.o $(BUILDDIR)/%.s.d: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ARCH_FLAGS) $(ASFLAGS) $(CPPFLAGS) -Wa,--MD,$@ -c $< -o $(@:%.d=%.o)


# Temp hack for libc
$(BUILDDIR)/libc/%.c.o $(BUILDDIR)/libc/%.c.d: $(SRCDIR)/libc/%.c
	@mkdir -p $(dir $@)
	$(CC) $(ARCH_FLAGS) $(CFLAGS) $(CPPFLAGS) -MMD -c $< -o $(@:%.d=%.o)

$(BUILDDIR)/libc/%.cpp.o $(BUILDDIR)/libc/%.cpp.d: $(SRCDIR)/libc/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(ARCH_FLAGS) $(CXXFLAGS) $(CPPFLAGS) -MMD -c $< -o $(@:%.d=%.o)

$(BUILDDIR)/libc/%.s.o $(BUILDDIR)/libc/%.s.d: $(SRCDIR)/libc/%.s
	@mkdir -p $(dir $@)
	$(AS) $(ARCH_FLAGS) $(ASFLAGS) $(CPPFLAGS) -Wa,--MD,$@ -c $< -o $(@:%.d=%.o)
