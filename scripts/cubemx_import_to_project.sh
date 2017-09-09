#!/bin/bash
set -euo pipefail

# functions ###################################

usage() {
	cat <<EOF
Example:
	$0 cube_mx_project 01_template_project
will do:
$(precmd=echo work cube_mx_project 01_template_project | sed 's/^/\t/')
EOF
}

read_continue() {
	local a
	while read -p "Continue? [y/n]"$'\n' a; do
		case "$a" in
			y|Y) return 0; ;;
			n|N) return 1; ;;
			*) echo "Please type exactly 'y' or 'n'."; ;;
		esac
	done
}

# main #############################################


diff_main_c() {
	cat <<EOF
--- main.c	2017-08-26 14:55:44.644387411 +0200
+++ Src/main.c	2017-08-26 15:01:48.175762213 +0200
@@ -344,41 +350,6 @@

- /**
-  * @brief  This function is executed in case of error occurrence.
-  * @param  None
-  * @retval None
-  */
-void _Error_Handler(char * file, int line)
-{
-  /* USER CODE BEGIN Error_Handler_Debug */
-  /* User can add his own implementation to report the HAL error return state */
-  while(1)
-  {
-  }
-  /* USER CODE END Error_Handler_Debug */
-}
-
-#ifdef USE_FULL_ASSERT
-
-/**
-   * @brief Reports the name of the source file and the source line number
-   * where the assert_param error has occurred.
-   * @param file: pointer to the source file name
-   * @param line: assert_param error line source number
-   * @retval None
-   */
-void assert_failed(uint8_t* file, uint32_t line)
-{
-  /* USER CODE BEGIN 6 */
-  /* User can add his own implementation to report the file name and line number,
-    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
-  /* USER CODE END 6 */
-
-}
-
-#endif

EOF
}

diff_main_h() {
	cat <<EOF
--- Inc/main.h	2017-08-26 15:04:53.719675397 +0200
+++ main.h	2017-08-26 14:55:44.647720579 +0200
@@ -63,8 +63,9 @@
 
-void _Error_Handler(char *, int);
-
-#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
+#include "stm32_assert.h"
+#define Error_Handler() assert_param(false)

EOF
}

work() {
        local pre=${precmd:-} s=$1 d=$2/cubemx
	$pre mkdir -p $d
        $pre cp -f $s/Src/* $s/Inc/* $d/
        $pre mv $d/main.c $d/mx_main.c
        $pre mv $d/main.h $d/mx_main.h
	# remove _Error_Handler(__FILE__,__LINE__)
	for f in $(grep -l -e '_Error_Handler(__FILE__, __LINE__);' $d); do
		f=$d/$(basename $f)
		$pre sed -i $f -e 's:_Error_Handler(__FILE__, __LINE__);:assert_param(false);:'
	done
	# fix include "main.h" in hal_conf.h
	for f in $(echo $d/stm32*xx_hal_conf.h); do
		f=$d/$(basename $f)
		$pre sed -i $f -e 's:^#include "main.h":// #include "main.h":'
	done
}

if [ $# -ne 2 ]; then usage; exit 1; fi

s=$1 d=$2/cubemx

if [ -e "$d" ] && [ ! -z "$(ls -A "$d")" ]; then
	ls $d
	echo "This script will erase everything in $d folder."
	read_continue || exit 1
fi

cat <<EOF
This script will do:
$(precmd=echo work "$@")
EOF
read_continue || exit 1

set -x
work "$@"

