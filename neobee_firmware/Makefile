
release:
	@echo "Releasing version"
	@echo =================================
	@echo "Updating version header file"
	@echo =================================
	python generate_header_file.py
	@echo =================================
	@echo Cleaning Up
	@echo =================================
	pio run -t clean -s
	@echo =================================
	@echo Build all versions
	@echo =================================
	pio run -s

	@echo =================================
	@echo Updating firmware folder
	@echo =================================
	@rm -f firmware/neobee_wemos_latest-debug.bin
	@rm -f firmware/neobee_wemos_latest.bin
	@cp .pio/build/wemos_debug/firmware.bin firmware/neobee_wemos_latest-debug.bin
	@cp .pio/build/wemos_release/firmware.bin firmware/neobee_wemos_latest.bin
