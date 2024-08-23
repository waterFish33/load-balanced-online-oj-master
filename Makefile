.PHONY:all
all:
	@cd compile_server;\
	make;\
	cd -;\
	cd oj_server;\
	make;\
	cd -;
.PHONY:output
output:
	@mkdir -p output-release/compile_server;\
	mkdir -p output-release/oj_server;\
	cp -rf compile_server/temp output-release/compile_server;\
	cp -rf compile_server/compile_run_server output-release/compile_server;\
	cp -rf oj_server/question_html output-release/oj_server;\
	cp -rf oj_server/questions output-release/oj_server;\
	cp -rf oj_server/service_machine output-release/oj_server;\
	cp -rf oj_server/wwwroot output-release/oj_server;\
	cp -rf oj_server/oj_server output-release/oj_server;

.PHONY:clean
clean:
	@cd compile_server;\
	make clean;\
	cd -;\
	cd oj_server;\
	make clean;\
	cd -;\
	rm -rf output-release