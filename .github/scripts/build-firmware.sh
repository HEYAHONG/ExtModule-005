#!/usr/bin/env bash
#
# Build every firmware project under FW/ in numeric order and publish the
# resulting snapshots to the "testing" GitHub release.
#
# Classification (checked in this order):
#   * a project containing CMakeLists.txt  -> ESP-IDF (idf.py build)
#   * a project name containing "nuttx"    -> NuttX   (make)
#   * a project name containing "zephyr"   -> Zephyr  (cmake, one build per
#                                            numeric+letter subproject)
#
# The script is intentionally sequential: it does not use a GitHub Actions
# build matrix, so projects are always built in the same numeric order.
#
# A failing project (or Zephyr subproject) does not stop the remaining builds.
# Every failure is recorded and reported at the end of the script, which then
# exits non-zero so the workflow still fails.

set -euo pipefail

ROOT="${GITHUB_WORKSPACE:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)}"
FW_DIR="${ROOT}/FW"
DIST_DIR="${ROOT}/dist"
IDF_ENV="${FW_DIR}/esp-idf-env.sh"
ZEPHYR_BOARD="${ZEPHYR_BOARD:-esp32_devkitc/esp32/procpu}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

export IDF_TARGET="${IDF_TARGET:-esp32}"

RELEASE_READY=0
FAILED_ITEMS=()
FAILED_REASONS=()

mkdir -p "${DIST_DIR}"

log() {
    echo
    echo "==================== $* ===================="
}

die() {
    echo "error: $*" >&2
    exit 1
}

record_failure() {
    local item="$1"
    local reason="$2"

    FAILED_ITEMS+=("${item}")
    FAILED_REASONS+=("${reason}")
    echo "FAILED: ${item}: ${reason}" >&2
    echo "::error::${item}: ${reason}" >&2
}

prepare_release() {
    if [ "${GITHUB_EVENT_NAME:-}" = "pull_request" ]; then
        RELEASE_READY=1
        return 0
    fi

    if gh release view testing --repo "${GITHUB_REPOSITORY}" >/dev/null 2>&1; then
        RELEASE_READY=1
        return 0
    fi

    if gh release create testing \
        --repo "${GITHUB_REPOSITORY}" \
        --title "testing" \
        --notes "Latest firmware snapshot builds" \
        --target "${GITHUB_SHA}"; then
        RELEASE_READY=1
        return 0
    fi

    return 1
}

publish_asset() {
    local archive="$1"

    if [ "${GITHUB_EVENT_NAME:-}" = "pull_request" ]; then
        echo "Pull request build: not publishing ${archive}"
        return 0
    fi

    if [ "${RELEASE_READY}" -ne 1 ]; then
        echo "Testing release is not available: not publishing ${archive}" >&2
        return 0
    fi

    gh release upload testing "${archive}" \
        --repo "${GITHUB_REPOSITORY}" --clobber
}

# Create an archive from files matched by the supplied find expression.
# Paths are stored relative to base_dir, keeping subdirectories intact.
# CMakeFiles directories are build-system metadata and are always skipped.
archive_from_find() {
    local base_dir="$1"
    local archive="$2"
    shift 2

    local -a files=()
    mapfile -d '' files < <(
        find "${base_dir}" \
            -type d -name CMakeFiles -prune -o \
            -type f "$@" -printf '%P\0'
    )

    if [ "${#files[@]}" -eq 0 ]; then
        echo "error: No matching artifacts found under ${base_dir}" >&2
        return 1
    fi

    if ! printf '%s\0' "${files[@]}" |
        tar -C "${base_dir}" --null -T - -czf "${archive}"; then
        echo "error: Failed to create ${archive}" >&2
        return 1
    fi

    echo "Created ${archive} from ${#files[@]} file(s)"
}

package_esp_idf() {
    local name="$1"
    local dir="$2"
    local archive="${DIST_DIR}/${name}-snapshot.tar.gz"

    archive_from_find "${dir}/build" "${archive}" \
        '(' -name '*.bin' -o -name '*.elf' -o -name '*.map' ')' || return 1
    publish_asset "${archive}" || return 1
}

package_nuttx() {
    local name="$1"
    local dir="$2"
    local archive="${DIST_DIR}/${name}-snapshot.tar.gz"

    if [ ! -d "${dir}/nuttx" ]; then
        echo "error: NuttX output directory not found: ${dir}/nuttx" >&2
        return 1
    fi
    if [ ! -d "${dir}/boot" ]; then
        echo "error: NuttX boot directory not found: ${dir}/boot" >&2
        return 1
    fi

    # Only package the build outputs.  NuttX writes them into the nuttx/
    # directory (the raw ELF image has no extension, so it is added
    # explicitly), while boot/ holds the prebuilt bootloader and partition
    # table images.  Do not walk the whole source tree: it contains unrelated
    # binary test assets under nuttx/arch/.
    local -a files=()
    mapfile -d '' files < <(
        find "${dir}" -maxdepth 1 -type f \
            \( -name '*.bin' -o -name '*.elf' -o -name '*.map' \) \
            -printf '%P\0'
        find "${dir}/nuttx" -maxdepth 1 -type f \
            \( -name '*.bin' -o -name '*.elf' -o -name '*.map' \) \
            -printf 'nuttx/%f\0'
        if [ -f "${dir}/nuttx/nuttx" ]; then
            printf 'nuttx/nuttx\0'
        fi
        find "${dir}/boot" -maxdepth 1 -type f -name '*.bin' \
            -printf 'boot/%f\0'
    )

    if [ "${#files[@]}" -eq 0 ]; then
        echo "error: No matching artifacts found under ${dir}" >&2
        return 1
    fi

    if ! printf '%s\0' "${files[@]}" |
        tar -C "${dir}" --null -T - -czf "${archive}"; then
        echo "error: Failed to create ${archive}" >&2
        return 1
    fi

    echo "Created ${archive} from ${#files[@]} file(s)"
    publish_asset "${archive}" || return 1
}

package_zephyr() {
    local name="$1"
    local build_dir="$2"
    local archive="${DIST_DIR}/${name}-snapshot.tar.gz"

    archive_from_find "${build_dir}" "${archive}" \
        '(' -name '*.bin' -o -name '*.elf' -o -name '*.map' ')' || return 1
    publish_asset "${archive}" || return 1
}

build_esp_idf() {
    local name="$1"
    local dir="$2"

    log "ESP-IDF: ${name}"
    if ! (
        cd "${dir}" || exit 1
        # The repository requires the ESP-IDF environment to be sourced before
        # every build.  FW/esp-idf-env.sh is created by the workflow because
        # the developer helper is not tracked in the repository.
        # shellcheck disable=SC1090
        source "${IDF_ENV}" || exit 1
        idf.py build
    ); then
        record_failure "${name}" "ESP-IDF build failed"
        return 0
    fi

    if ! package_esp_idf "${name}" "${dir}"; then
        record_failure "${name}" "ESP-IDF packaging/publishing failed"
    fi
    return 0
}

build_nuttx() {
    local name="$1"
    local dir="$2"

    log "NuttX: ${name}"
    if ! (
        cd "${dir}" || exit 1
        # shellcheck disable=SC1090
        source "${IDF_ENV}" || exit 1
        make -j"$(nproc)"
    ); then
        record_failure "${name}" "NuttX build failed"
        return 0
    fi

    if ! package_nuttx "${name}" "${dir}"; then
        record_failure "${name}" "NuttX packaging/publishing failed"
    fi
    return 0
}

build_zephyr() {
    local name="$1"
    local dir="$2"
    local -a subprojects=()

    mapfile -t subprojects < <(
        find "${dir}" -mindepth 1 -maxdepth 1 -type d \
            -regextype posix-extended -regex '.*/[0-9]+[-_A-Za-z].*' \
            -printf '%f\n' |
            sort -V
    )

    if [ "${#subprojects[@]}" -eq 0 ]; then
        record_failure "${name}" "no numeric+letter subprojects found"
        return 0
    fi

    local sub
    for sub in "${subprojects[@]}"; do
        local full_name="${name}-${sub}"
        local build_dir="${dir}/build/${sub}"

        log "Zephyr: ${full_name}"
        if ! (
            cd "${dir}" || exit 1
            # shellcheck disable=SC1090
            source "${IDF_ENV}" || exit 1

            # ESP-IDF prepends its own Python environment.  Zephyr must use the
            # image's Zephyr environment (west, pyelftools, ...), so restore it
            # to the front of PATH after sourcing the ESP-IDF environment.
            export PATH="/opt/python/venv/bin:${PATH}"

            export ZEPHYR_BASE="${dir}/zephyr"
            export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
            if [ -z "${ZEPHYR_SDK_INSTALL_DIR:-}" ]; then
                ZEPHYR_SDK_INSTALL_DIR="$(
                    find /opt/toolchains -maxdepth 1 -type d \
                        -name 'zephyr-sdk-*' -print -quit 2>/dev/null || true
                )"
                if [ -z "${ZEPHYR_SDK_INSTALL_DIR}" ]; then
                    echo "error: Zephyr SDK not found under /opt/toolchains" >&2
                    exit 1
                fi
                export ZEPHYR_SDK_INSTALL_DIR
            fi

            rm -rf "${build_dir}" || exit 1
            # The applications are documented to be built with sysbuild.  Use
            # Zephyr's sysbuild CMake entry point directly so that both the
            # bootloader and the application are produced (and packaged).
            cmake -S "${ZEPHYR_BASE}/share/sysbuild" -B "${build_dir}" -GNinja \
                -DBOARD="${ZEPHYR_BOARD}" \
                -DAPP_DIR="${dir}/${sub}" \
                -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" || exit 1
            cmake --build "${build_dir}" --parallel "$(nproc)"
        ); then
            record_failure "${full_name}" "Zephyr build failed"
            continue
        fi

        if ! package_zephyr "${full_name}" "${build_dir}"; then
            record_failure "${full_name}" "Zephyr packaging/publishing failed"
            continue
        fi
    done

    return 0
}

if ! prepare_release; then
    record_failure "testing release" "could not find or create the release"
fi

log "Scanning firmware projects in ${FW_DIR}"
mapfile -t projects < <(
    find "${FW_DIR}" -mindepth 1 -maxdepth 1 -type d \
        -regextype posix-extended -regex '.*/[0-9]+-.+' \
        -printf '%f\n' |
        sort -V
)

if [ "${#projects[@]}" -eq 0 ]; then
    die "No firmware projects found in ${FW_DIR}"
fi

for project in "${projects[@]}"; do
    project_dir="${FW_DIR}/${project}"

    if [ -f "${project_dir}/CMakeLists.txt" ]; then
        build_esp_idf "${project}" "${project_dir}"
    elif [[ "${project}" == *nuttx* ]]; then
        build_nuttx "${project}" "${project_dir}"
    elif [[ "${project}" == *zephyr* ]]; then
        build_zephyr "${project}" "${project_dir}"
    else
        echo "warning: cannot classify ${project}, skipping" >&2
    fi
done

log "Generated snapshots"
find "${DIST_DIR}" -maxdepth 1 -type f -printf '%f\n' | sort

log "Build summary"
if [ "${#FAILED_ITEMS[@]}" -gt 0 ]; then
    echo "The following build steps failed:" >&2
    for i in "${!FAILED_ITEMS[@]}"; do
        echo "  - ${FAILED_ITEMS[$i]}: ${FAILED_REASONS[$i]}" >&2
    done
    exit 1
fi

echo "All firmware builds completed successfully."
