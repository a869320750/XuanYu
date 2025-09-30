set -euo pipefail

# 把本地文件上传到远端 Web 根目录，并尝试正确移动到目标位置（会检测是否可写或是否有无密码 sudo）
REMOTE_HOST="${REMOTE_HOST:-106.15.42.105}"
REMOTE_USER="${REMOTE_USER:-admin}"
REMOTE_PORT="${REMOTE_PORT:-22}"
LOCAL_FILE="${1:-test.py}"
REMOTE_TARGET_PATH="${REMOTE_TARGET_PATH:-/var/www/html/${LOCAL_FILE##*/}}"

if [ ! -f "$LOCAL_FILE" ]; then
  echo "ERROR: 本地文件不存在: $LOCAL_FILE"
  exit 2
fi

basename_local="$(basename "$LOCAL_FILE")"
remote_tmp="/tmp/${basename_local}"
remote_dir="$(dirname "$REMOTE_TARGET_PATH")"

echo "Uploading $LOCAL_FILE -> ${REMOTE_USER}@${REMOTE_HOST}:${remote_tmp} ..."
scp -P "$REMOTE_PORT" "$LOCAL_FILE" "${REMOTE_USER}@${REMOTE_HOST}:${remote_tmp}"

echo "Checking remote permissions..."
check_out=$(ssh -p "$REMOTE_PORT" "${REMOTE_USER}@${REMOTE_HOST}" "bash -lc \"if [ -w '${REMOTE_TARGET_PATH}' ] || [ -w '${remote_dir}' ]; then echo CAN_WRITE; elif sudo -n true 2>/dev/null; then echo SUDO_NOPASS; else echo NO_SUDO; fi\"")

echo "Remote check result: $check_out"

if [ "$check_out" = "CAN_WRITE" ]; then
  echo "Remote directory writable by ${REMOTE_USER}, moving without sudo..."
  ssh -p "$REMOTE_PORT" "${REMOTE_USER}@${REMOTE_HOST}" "mv -f '${remote_tmp}' '${REMOTE_TARGET_PATH}' && chmod 644 '${REMOTE_TARGET_PATH}'"
  echo "OK: 发布完成： http://${REMOTE_HOST}/$(basename "${REMOTE_TARGET_PATH}")"
  echo "通过下面命令下载："
  echo "wget http://${REMOTE_HOST}/$(basename "${REMOTE_TARGET_PATH}") -O $(basename "${REMOTE_TARGET_PATH}") "
  exit 0
elif [ "$check_out" = "SUDO_NOPASS" ]; then
  echo "Remote has passwordless sudo, moving with sudo..."
  ssh -p "$REMOTE_PORT" "${REMOTE_USER}@${REMOTE_HOST}" "sudo mv -f '${remote_tmp}' '${REMOTE_TARGET_PATH}' && sudo chmod 644 '${REMOTE_TARGET_PATH}'"
  echo "OK: 发布完成： http://${REMOTE_HOST}/$(basename "${REMOTE_TARGET_PATH}")"
  echo "通过下面命令下载："
  echo "wget http://${REMOTE_HOST}/$(basename "${REMOTE_TARGET_PATH}") -O $(basename "${REMOTE_TARGET_PATH}") "
  exit 0
else
  echo "ERROR: 远端目录不可写，且当前用户没有免密码 sudo。"
  echo "选项："
  echo "  1) 在远端将文件从 /tmp 移动到目标（需 sudo），例如："
  echo "       ssh ${REMOTE_USER}@${REMOTE_HOST} 'sudo mv /tmp/${basename_local} ${REMOTE_TARGET_PATH} && sudo chmod 644 ${REMOTE_TARGET_PATH}'"
  echo "  2) 使用可以写入目标目录或有 sudo 权限的用户账号（修改 REMOTE_USER 或配置免密 sudo）。"
  echo "  3) 修改 REMOTE_TARGET_PATH 指向当前用户可写的 webroot。"
  exit 3
fi