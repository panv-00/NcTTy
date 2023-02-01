default:
	@+make -C build

clean:
	@rm -f build/*.o
	@rm -f ncrev
	@echo "Clean!"
