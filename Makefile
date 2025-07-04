CC = gcc
MUSL_CC = musl-gcc
CFLAGS = -static -Os -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -w
LDFLAGS = -s
SOURCES = naucanh.c utilities.c crypt-md5.c crypt-sha256.c crypt-sha512.c alg-md5.c alg-sha256.c alg-sha512.c

all: musl

naucanh: $(SOURCES)
	@echo "Compiling with gcc..."
	$(CC) $(CFLAGS) -o naucanh $(SOURCES) $(LDFLAGS)
	@strip --strip-all naucanh 2>/dev/null || true

musl: $(SOURCES)
	@if command -v $(MUSL_CC) >/dev/null 2>&1; then \
		echo "Compiling with musl-gcc..."; \
		$(MUSL_CC) $(CFLAGS) -o naucanh $(SOURCES) $(LDFLAGS); \
	else \
		echo "musl-gcc not found, using gcc..."; \
		$(CC) $(CFLAGS) -o naucanh $(SOURCES) $(LDFLAGS); \
	fi
	@strip --strip-all naucanh 2>/dev/null || true

clean:
	rm -f naucanh *.o

test: naucanh
	@echo "=== Testing All Hash Functions ==="
	@echo "SHA-512:"
	@./naucanh -6 1CCHezNlaiEUpRNw 1231234
	@echo ""
	@echo "SHA-256:"
	@./naucanh -5 testsalt testpassword
	@echo ""
	@echo "MD5:"
	@./naucanh -1 testsalt testpassword

verify: naucanh
	@echo "=== Verification Test ==="
	@EXPECTED='$6$1CCHezNlaiEUpRNw$bf3jd2VJgQ9B91SjVjFBQosiKOgv3DJyeVguo8.EVXkxNzJEPvpSyenNmZ4xoke.N0x2V1PPLRHrM8Msj8Kt8.'; \
	ACTUAL=$(./naucanh -6 1CCHezNlaiEUpRNw 1231234); \
	echo "Expected: $EXPECTED"; \
	echo "Actual:   $ACTUAL"; \
	if [ "$ACTUAL" = "$EXPECTED" ]; then \
		echo "✅ SUCCESS! Hash matches perfectly!"; \
	else \
		echo "❌ FAILED! Hash mismatch"; \
	fi

debug: $(SOURCES)
	@echo "Compiling with debug symbols..."
	$(CC) -static -g -O0 -o naucanh-debug $(SOURCES)
	@echo "Debug binary: naucanh-debug"

.PHONY: all musl clean test verify debug