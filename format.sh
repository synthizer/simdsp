set -e
for f in $(find src include tests bench -path src/data -prune -o -type f -print); do
	clang-format-10 -style=file -i $f
done
