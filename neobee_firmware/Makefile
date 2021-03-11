
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

	@echo =================================x
	@echo Build all versions
	@echo =================================
	pio run -s

	@echo =================================
	@echo Updating firmware folder
	@echo =================================
	python save_firmware.py