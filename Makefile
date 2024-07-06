CC = gcc
CFLAGS = -Wall $(COPTIONS)
LDFLAGS =
LOADLIBES =
LDLIBS = -lm -lSDL2 -lSDL2_image -lSDL2_ttf

TARGET = sdl_window
OBJS = main.o player.o bg.o object.o boss.o  # boss.o を追加

# デバッグ用ターゲット
debug: CPPFLAGS =
debug: COPTIONS = -g3
debug: $(TARGET)

# リリース用ターゲット
release: CPPFLAGS = -DNDEBUG
release: COPTIONS = -O3
release: $(TARGET)

# コンパイルとリンクのルール
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

# 中間オブジェクトファイルの生成
%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# レポート作成用ターゲット
%.pdf: %.dvi
	dvipdfmx $<

# クリーンアップ
.PHONY: clean
clean:
	@rm -f $(TARGET) $(OBJS) core *~
