
.PHONY: lint
lint:
	find ./ -type f \( -iname \*.h -o -iname \*.cpp \) | xargs clang-format -n -Werror

.PHONY: fix
fix:
	find ./ -type f \( -iname \*.h -o -iname \*.cpp \) | xargs clang-format -i