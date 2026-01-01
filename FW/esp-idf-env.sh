#!/bin/bash

#检查工具是否存在,$1为待检查的工具名。
function CheckTool
{
	[  -n "$1"  ] ||
	{
		echo -e  "\033[41;37mCheckTool 参数错误!!\033[40;37m";
		return 255;
	};
	ToolPath=`which $1`;
	[ -e "$ToolPath" ] ||
	{
		 echo -e "\033[41;37m$1 不存在，请先安装此工具\033[40;37m";
		 return 255;
	};
	return 0;
}

#检查必要的工具
CheckTool git
[ $? -eq 0 ] || exit;
CheckTool install
[ $? -eq 0 ] || exit;
CheckTool patch
[ $? -eq 0 ] || exit;
CheckTool find
[ $? -eq 0 ] || exit;
CheckTool dirname
[ $? -eq 0 ] || exit;
CheckTool readlink
[ $? -eq 0 ] || exit;
CheckTool ln
[ $? -eq 0 ] || exit;
CheckTool sed
[ $? -eq 0 ] || exit;
CheckTool python3
[ $? -eq 0 ] || exit;
CheckTool grep
[ $? -eq 0 ] || exit;


self_path=""

# shellcheck disable=SC2128  # ignore array expansion warning
if [ -n "${BASH_SOURCE-}" ]
then
self_path="${BASH_SOURCE}"
elif [ -n "${ZSH_VERSION-}" ]
then
self_path="${(%):-%x}"
else
echo -e "\033[41;37m不能获取工作目录\033[40;37m";
return 1
fi

# shellcheck disable=SC2169,SC2169,SC2039  # unreachable with 'dash'
if [[ "$OSTYPE" == "darwin"* ]]; then
# convert possibly relative path to absolute
script_dir="$(realpath_int "${self_path}")"
# resolve any ../ references to make the path shorter
script_dir="$(cd "${script_dir}" || exit 1; pwd)"
else
# convert to full path and get the directory name of that
script_name="$(readlink -f "${self_path}")"
script_dir="$(dirname "${script_name}")"
fi

ROOT_PATH="${script_dir}";
SDK_PATH=${ROOT_PATH}/3rdparty/esp-idf/

echo -e  "\033[44;37mesp-idf目录：${SDK_PATH}\033[40;37m";

echo -e  "\033[44;37m正在初始化SDK环境\033[40;37m";
# 更换下载地址
export IDF_GITHUB_ASSETS="dl.espressif.com/github_assets"


pushd ${SDK_PATH}
function InstallFailure
{
	popd
	echo -e "初始化失败,重新执行。"
	exit 1;
}
git submodule update --init --recursive --jobs 8
[ $? -eq 0 ] || InstallFailure
./install.sh
[ $? -eq 0 ] || InstallFailure
source ./export.sh
popd

echo -e  "\033[44;37mSDK环境初始化完毕!\033[40;37m";
exec ${SHELL}
