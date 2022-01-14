project(document_editor)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( document_editor document_editor 
	document_editor.cpp )
target_include_directories( document_editor PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( document_editor ${CMAKE_SOURCE_DIR}/../ricardian )