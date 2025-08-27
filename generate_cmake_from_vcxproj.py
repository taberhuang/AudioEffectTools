#!/usr/bin/env python3
"""
Convert Visual Studio 2022 JUCE projects (.vcxproj) to CMake.
Automatically detects and configures JUCE-specific requirements.
Enhanced version with smart JUCE path detection from vcxproj files.
"""

import os
import sys
import glob
import xml.etree.ElementTree as ET
from pathlib import Path
import re
import subprocess

MSBUILD_NS = "http://schemas.microsoft.com/developer/msbuild/2003"

# Common Windows system libraries needed by JUCE
JUCE_WINDOWS_LIBS = [
    "kernel32.lib", "user32.lib", "gdi32.lib", "winspool.lib",
    "comdlg32.lib", "advapi32.lib", "shell32.lib", "ole32.lib",
    "oleaut32.lib", "uuid.lib", "odbc32.lib", "odbccp32.lib",
    "winmm.lib", "wsock32.lib", "wininet.lib", "version.lib",
    "ws2_32.lib", "Shlwapi.lib", "Imm32.lib"
]

def check_and_init_juce_submodule() -> bool:
    """Check and initialize JUCE submodule if needed."""
    root = Path.cwd()
    gitmodules_file = root / ".gitmodules"
    juce_dir = root / "JUCE"
    juce_cmake = juce_dir / "CMakeLists.txt"
    
    # Check if .gitmodules exists and contains JUCE
    if not gitmodules_file.exists():
        print("[INFO] No .gitmodules found. Skipping submodule check.")
        return True
    
    try:
        gitmodules_content = gitmodules_file.read_text(encoding='utf-8')
        if "JUCE" not in gitmodules_content:
            print("[INFO] JUCE not found in .gitmodules. Skipping submodule check.")
            return True
    except Exception as e:
        print(f"[WARN] Could not read .gitmodules: {e}")
        return True
    
    # Check if JUCE submodule is properly initialized
    if juce_cmake.exists():
        print("[INFO] JUCE submodule already exists and initialized.")
        return True
    
    print("[INFO] JUCE submodule not found. Initializing with shallow clone...")
    
    try:
        # Initialize JUCE submodule with shallow clone
        result = subprocess.run([
            "git", "submodule", "update", "--init", "--depth", "1", "JUCE"
        ], capture_output=True, text=True, timeout=300)
        
        if result.returncode == 0:
            print("[SUCCESS] JUCE submodule initialized successfully.")
            return True
        else:
            print(f"[ERROR] Failed to initialize JUCE submodule:")
            print(f"        {result.stderr}")
            return False
            
    except subprocess.TimeoutExpired:
        print("[ERROR] JUCE submodule initialization timed out (5 minutes).")
        return False
    except FileNotFoundError:
        print("[ERROR] git command not found. Please install Git.")
        return False
    except Exception as e:
        print(f"[ERROR] Unexpected error during JUCE submodule initialization: {e}")
        return False

def strip_ns(tag: str) -> str:
    """Strip XML namespace from tag."""
    if tag.startswith("{"):
        return tag.split("}", 1)[1]
    return tag

def normalize_path(path_str: str, base_dir: Path) -> str:
    """Normalize MSBuild paths to CMake-compatible paths."""
    if not path_str:
        return ""
    
    # Replace common MSBuild macros
    replacements = {
        "$(ProjectDir)": str(base_dir) + "/",
        "$(SolutionDir)": str(base_dir.parent) + "/",
        "$(Configuration)": "${CMAKE_BUILD_TYPE}",
        "$(Platform)": "x64",
        "$(PlatformTarget)": "x64",
    }
    
    normalized = path_str
    for k, v in replacements.items():
        normalized = normalized.replace(k, v)
    
    # Remove inherited macros
    if "%(" in normalized:
        parts = [p for p in normalized.split(";") if "%(" not in p]
        normalized = ";".join(parts)
    
    # Handle semicolon-separated lists
    if ";" in normalized:
        paths = []
        for p in normalized.split(";"):
            p = p.strip()
            if not p:
                continue
            # Keep CMake variables as-is
            if p.startswith("$"):
                paths.append(p)
            else:
                # Convert to absolute path if relative
                if not os.path.isabs(p):
                    p = str((base_dir / p).resolve())
                p = p.replace("\\", "/")
                paths.append(p)
        return ";".join(paths)
    
    # Single path
    p = normalized.strip()
    if not p:
        return ""
    if p.startswith("$"):
        return p
    if not os.path.isabs(p):
        p = str((base_dir / p).resolve())
    return p.replace("\\", "/")

def matches_condition(cond: str, config: str, platform: str = "x64") -> bool:
    """Check if an MSBuild condition matches the given configuration."""
    if not cond:
        return True
    
    c = cond.replace(" ", "")
    
    # Handle common condition patterns
    patterns = [
        (r"'\$\(Configuration\)\|\$\(Platform\)'=='(.+)\|(.+)'", lambda m: m.group(1).lower() == config.lower() and m.group(2).lower() == platform.lower()),
        (r"'\$\(Configuration\)'=='(.+)'", lambda m: m.group(1).lower() == config.lower()),
    ]
    
    for pattern, checker in patterns:
        match = re.match(pattern, c)
        if match:
            return checker(match)
    
    return False

def extract_juce_path_from_includes(include_dirs: list, base_dir: Path) -> tuple:
    """
    Extract JUCE path from include directories.
    Returns (juce_path, juce_modules_path, is_submodule)
    """
    juce_path = None
    juce_modules_path = None
    is_submodule = False
    
    # First, check if JUCE is a local subdirectory  
    local_juce_dir = base_dir / "JUCE"
    if local_juce_dir.exists() and (local_juce_dir / "modules").exists():
        juce_path = local_juce_dir
        juce_modules_path = local_juce_dir / "modules"
        is_submodule = True
        return juce_path, juce_modules_path, is_submodule
    
    for inc_dir in include_dirs:
        # Look for paths containing JUCE/modules or similar
        if "JUCE" in inc_dir and "modules" in inc_dir:
            # Try to extract the JUCE root path
            parts = Path(inc_dir).parts
            try:
                juce_idx = parts.index("JUCE")
                modules_idx = parts.index("modules")
                
                if modules_idx == juce_idx + 1:
                    # Standard JUCE structure: .../JUCE/modules
                    juce_path = Path(*parts[:juce_idx+1])
                    juce_modules_path = Path(*parts[:modules_idx+1])
                    
                    # Check if JUCE is a subdirectory of the project
                    try:
                        rel_path = juce_path.relative_to(base_dir)
                        is_submodule = True
                    except ValueError:
                        is_submodule = False
                    
                    break
            except (ValueError, IndexError):
                continue
    
    return juce_path, juce_modules_path, is_submodule

def detect_juce_modules(project_root: Path) -> list:
    """Detect which JUCE modules are used in the project."""
    modules = []
    juce_library_code = project_root / "JuceLibraryCode"
    
    if juce_library_code.exists():
        # Check both .cpp and .c files
        for source_file in list(juce_library_code.glob("include_juce_*.cpp")) + list(juce_library_code.glob("include_juce_*.c")):
            # Extract module name from filename
            module_name = source_file.stem.replace("include_", "")
            
            # Skip special files that are not independent modules
            special_suffixes = ["_ara", "_lv2_libs", "_CompilationTime", "_Harfbuzz", "_Sheenbidi"]
            if any(module_name.endswith(suffix) for suffix in special_suffixes):
                # These are parts of other modules, extract base module name
                base_module = module_name
                for suffix in special_suffixes:
                    base_module = base_module.replace(suffix, "")
                
                # Only add the base module if it's a valid JUCE module
                if base_module and base_module not in modules:
                    modules.append(base_module)
            else:
                # Regular module
                if module_name not in modules:
                    modules.append(module_name)
    
    return modules

def detect_juce_structure(base_dir: Path, configs: dict) -> dict:
    """
    Detect JUCE structure from project configuration.
    Returns dict with JUCE configuration details.
    """
    juce_info = {
        "has_juce": False,
        "juce_path": None,
        "juce_modules_path": None,
        "is_submodule": False,
        "modules": [],
        "use_modern_cmake": False,
        "juce_library_code": False
    }
    
    # Check for JuceLibraryCode (Projucer-generated project)
    # JuceLibraryCode is in the project root, not in Builds/VisualStudio2022
    project_root = base_dir.parent.parent if "Builds" in str(base_dir) else base_dir
    juce_library_code = project_root / "JuceLibraryCode"
    if juce_library_code.exists():
        juce_info["has_juce"] = True
        juce_info["juce_library_code"] = True
        juce_info["modules"] = detect_juce_modules(project_root)
        
        # Try to find JUCE path from include directories
        for config_name, cfg in configs.items():
            if cfg["include_dirs"]:
                juce_path, modules_path, is_sub = extract_juce_path_from_includes(
                    cfg["include_dirs"], project_root
                )
                if juce_path:
                    juce_info["juce_path"] = juce_path
                    juce_info["juce_modules_path"] = modules_path
                    juce_info["is_submodule"] = is_sub
                    break
        
        # For projects with JuceLibraryCode, use traditional approach
        # Modern CMake API is for projects built from scratch with JUCE CMake
        juce_info["use_modern_cmake"] = False
        print(f"[INFO] Using traditional CMake approach for Projucer-generated project")
    
    return juce_info

def is_juce_project(vcxproj_path: Path) -> bool:
    """Check if this is a JUCE project."""
    base_dir = vcxproj_path.parent
    
    # Check for JuceLibraryCode directory
    if (base_dir / "JuceLibraryCode").exists():
        return True
    
    # Check for JUCE header includes in the project
    if vcxproj_path.exists():
        content = vcxproj_path.read_text(encoding='utf-8', errors='ignore')
        if 'JuceHeader.h' in content or 'JuceLibraryCode' in content:
            return True
    
    return False

def parse_vcxproj(vcxproj_path: Path) -> dict:
    """Parse a .vcxproj file and extract build information."""
    
    base_dir = vcxproj_path.parent
    project_name = vcxproj_path.stem
    
    # Sanitize project name for CMake (replace spaces and special chars)
    cmake_target_name = re.sub(r'[^a-zA-Z0-9_]', '_', project_name)
    
    ns = {"msb": MSBUILD_NS}
    
    try:
        tree = ET.parse(vcxproj_path)
        root = tree.getroot()
    except ET.ParseError as ex:
        print(f"[ERROR] Failed to parse {vcxproj_path}: {ex}")
        return {}
    
    print(f"[INFO] Parsing {vcxproj_path.name}")
    
    # Detect if this is a JUCE project
    is_juce = is_juce_project(vcxproj_path)
    if is_juce:
        print(f"[INFO] Detected JUCE project: {project_name}")
    
    # Collect source files
    sources = set()
    resources = set()
    
    for item_group in root.findall(".//msb:ItemGroup", ns):
        for item in item_group:
            tag = strip_ns(item.tag)
            
            if tag in ("ClCompile", "ClInclude", "ResourceCompile"):
                include_attr = item.attrib.get("Include")
                if not include_attr:
                    continue
                
                # Check if excluded from build
                excluded = False
                for exc in item.findall("msb:ExcludedFromBuild", ns):
                    if exc.text and exc.text.strip().lower() in ("true", "1"):
                        cond = exc.attrib.get("Condition", "")
                        if not cond or matches_condition(cond, "Debug") or matches_condition(cond, "Release"):
                            excluded = True
                            break
                
                if excluded:
                    continue
                
                include_path = normalize_path(include_attr, base_dir)
                
                if tag == "ClCompile":
                    # Add all compilable source files (including .c files for SheenBidi)
                    if include_path.endswith(('.cpp', '.cc', '.cxx', '.c', '.m', '.mm')):
                        sources.add(include_path)
                elif tag == "ResourceCompile":
                    if include_path.endswith('.rc'):
                        resources.add(include_path)
    
    # Parse configuration-specific settings
    configs = {}
    
    for item_def in root.findall(".//msb:ItemDefinitionGroup", ns):
        cond = item_def.attrib.get("Condition", "")
        
        # Determine which configuration this applies to
        config_name = None
        if matches_condition(cond, "Debug"):
            config_name = "Debug"
        elif matches_condition(cond, "Release"):
            config_name = "Release"
        else:
            continue  # Skip non-matching conditions
        
        if config_name not in configs:
            configs[config_name] = {
                "include_dirs": [],
                "defines": [],
                "lib_dirs": [],
                "libs": [],
                "compile_options": [],
                "link_options": []
            }
        
        cfg = configs[config_name]
        
        # Parse ClCompile settings
        cl = item_def.find("msb:ClCompile", ns)
        if cl is not None:
            # Include directories
            inc_dirs = cl.find("msb:AdditionalIncludeDirectories", ns)
            if inc_dirs is not None and inc_dirs.text:
                dirs = normalize_path(inc_dirs.text, base_dir).split(";")
                cfg["include_dirs"].extend([d for d in dirs if d])
            
            # Preprocessor definitions
            defs = cl.find("msb:PreprocessorDefinitions", ns)
            if defs is not None and defs.text:
                for d in defs.text.split(";"):
                    d = d.strip()
                    if d and "%(" not in d:
                        cfg["defines"].append(d)
            
            # Runtime library
            runtime = cl.find("msb:RuntimeLibrary", ns)
            if runtime is not None and runtime.text:
                rt = runtime.text.strip()
                if rt == "MultiThreadedDebugDLL":
                    cfg["compile_options"].append("/MDd")
                elif rt == "MultiThreadedDLL":
                    cfg["compile_options"].append("/MD")
                elif rt == "MultiThreadedDebug":
                    cfg["compile_options"].append("/MTd")
                elif rt == "MultiThreaded":
                    cfg["compile_options"].append("/MT")
        
        # Parse Link settings
        link = item_def.find("msb:Link", ns)
        if link is not None:
            # Additional dependencies
            deps = link.find("msb:AdditionalDependencies", ns)
            if deps is not None and deps.text:
                for lib in deps.text.split(";"):
                    lib = lib.strip()
                    if lib and "%(" not in lib:
                        cfg["libs"].append(lib)
            
            # Library directories
            lib_dirs = link.find("msb:AdditionalLibraryDirectories", ns)
            if lib_dirs is not None and lib_dirs.text:
                dirs = normalize_path(lib_dirs.text, base_dir).split(";")
                cfg["lib_dirs"].extend([d for d in dirs if d])
            
            # Subsystem
            subsystem = link.find("msb:SubSystem", ns)
            if subsystem is not None and subsystem.text:
                if subsystem.text.strip().lower() == "windows":
                    cfg["link_options"].append("WIN32")
    
    # Combine sources and resources
    all_sources = list(sources | resources)
    all_sources.sort()
    
    # Detect JUCE structure
    juce_info = {}
    if is_juce:
        juce_info = detect_juce_structure(base_dir, configs)
        if juce_info["juce_path"]:
            print(f"[INFO] Found JUCE at: {juce_info['juce_path']}")
            if juce_info["is_submodule"]:
                print(f"[INFO] JUCE is a submodule/subdirectory of the project")
            if juce_info["use_modern_cmake"]:
                print(f"[INFO] JUCE has modern CMake support")
    
    return {
        "name": cmake_target_name,  # Use sanitized name for CMake
        "original_name": project_name,  # Keep original for display
        "sources": all_sources,
        "configs": configs,
        "is_juce": is_juce,
        "juce_info": juce_info,
        "base_dir": base_dir
    }

def generate_cmakelists(projects: list[dict]) -> str:
    """Generate CMakeLists.txt content from parsed projects."""
    
    lines = []
    
    # CMake header
    lines.append("cmake_minimum_required(VERSION 3.16)")
    
    # Determine project name and languages
    project_name = "JuceProject" if any(p.get("is_juce") for p in projects) else "VSProject"
    # JUCE projects need both C and CXX for SheenBidi
    languages = "C CXX" if any(p.get("is_juce") for p in projects) else "CXX"
    lines.append(f"project({project_name} VERSION 1.0.0 LANGUAGES {languages})")
    lines.append("")
    
    # C++ Standard
    lines.append("# C++ Standard")
    lines.append("set(CMAKE_CXX_STANDARD 17)")
    lines.append("set(CMAKE_CXX_STANDARD_REQUIRED ON)")
    lines.append("set(CMAKE_CXX_EXTENSIONS OFF)")
    lines.append("")
    
    # Output directories
    lines.append("# Output directories")
    lines.append("set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})")
    lines.append("set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})")
    lines.append("set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})")
    lines.append("")
    
    # Check if any project is JUCE and determine JUCE configuration
    has_juce = any(p.get("is_juce") for p in projects)
    juce_configs = [p.get("juce_info", {}) for p in projects if p.get("is_juce")]
    
    # Find the best JUCE configuration
    juce_config = None
    if juce_configs:
        # Prefer the one with detected JUCE path
        for cfg in juce_configs:
            if cfg.get("juce_path"):
                juce_config = cfg
                break
        if not juce_config:
            juce_config = juce_configs[0]
    
    # Windows-specific settings
    lines.append("# Windows-specific settings")
    lines.append("if(WIN32)")
    lines.append("    add_compile_definitions(WIN32 _WINDOWS _CRT_SECURE_NO_WARNINGS)")
    lines.append("    set(CMAKE_MSVC_RUNTIME_LIBRARY \"MultiThreaded$<$<CONFIG:Debug>:Debug>DLL\")")
    
    if has_juce:
        lines.append("    ")
        lines.append("    # JUCE-specific compiler flags")
        lines.append("    add_compile_options(")
        lines.append("        /bigobj     # Required for JUCE's large object files")
        lines.append("        /utf-8      # Fix character encoding warnings")
        lines.append("        /MP         # Multi-processor compilation for faster builds")
        lines.append("    )")
    
    lines.append("endif()")
    lines.append("")
    
    # JUCE configuration
    if has_juce and juce_config:
        lines.append("# JUCE configuration")
        
        if juce_config.get("use_modern_cmake") and juce_config.get("is_submodule"):
            # Modern JUCE with CMake support as submodule
            rel_path = juce_config["juce_path"].relative_to(Path.cwd())
            lines.append(f"# Using modern JUCE CMake support")
            lines.append(f"add_subdirectory(\"{rel_path}\" JUCE)")
            lines.append("")
        elif juce_config.get("juce_modules_path"):
            # Traditional approach with direct module path
            if juce_config.get("is_submodule"):
                # JUCE is a submodule - use relative path
                try:
                    rel_path = juce_config["juce_modules_path"].relative_to(Path.cwd())
                    lines.append(f"set(JUCE_MODULE_PATH \"${{CMAKE_CURRENT_SOURCE_DIR}}/{str(rel_path).replace(os.sep, '/')}\")")
                except ValueError:
                    # Can't make relative, use absolute
                    lines.append(f"set(JUCE_MODULE_PATH \"{str(juce_config['juce_modules_path']).replace(os.sep, '/')}\")")
            else:
                # JUCE is external - use absolute path or allow override
                default_path = str(juce_config["juce_modules_path"]).replace(os.sep, "/")
                lines.append(f"set(JUCE_MODULE_PATH \"{default_path}\" CACHE PATH \"Path to JUCE modules\")")
        else:
            # Fallback - let user specify
            lines.append("# JUCE path not detected from vcxproj - please set manually")
            lines.append("set(JUCE_MODULE_PATH \"C:/JUCE/modules\" CACHE PATH \"Path to JUCE modules\")")
            lines.append("message(WARNING \"JUCE path not detected. Please set JUCE_MODULE_PATH to your JUCE/modules directory\")")
        
        lines.append("")
    
    target_names = []
    
    for proj in projects:
        name = proj["name"]
        target_names.append(name)
        sources = proj["sources"]
        configs = proj.get("configs", {})
        is_juce = proj.get("is_juce", False)
        juce_info = proj.get("juce_info", {})
        
        lines.append(f"# Target: {name}")
        lines.append("#" + "="*60)
        
        # Check if it's a Windows GUI app
        is_win32 = any("WIN32" in cfg.get("link_options", []) 
                      for cfg in configs.values())
        
        # For modern JUCE CMake support
        if is_juce and juce_info.get("use_modern_cmake"):
            lines.append(f"# Using modern JUCE CMake API")
            lines.append(f"juce_add_gui_app({name}")
            lines.append(f"    PRODUCT_NAME \"{name}\"")
            if sources:
                lines.append("    SOURCES")
                for src in sources:
                    # Skip JUCE module sources
                    if "include_juce_" not in src:
                        lines.append(f"        \"{src}\"")
            lines.append(")")
            lines.append("")
            
            # Link JUCE modules
            if juce_info.get("modules"):
                lines.append(f"target_link_libraries({name}")
                lines.append("    PRIVATE")
                for module in juce_info["modules"]:
                    lines.append(f"        juce::{module}")
                lines.append(")")
                lines.append("")
        else:
            # Traditional approach
            if sources:
                lines.append(f"add_executable({name}")
                if is_win32:
                    lines.append("    WIN32  # Windows GUI application")
                for src in sources:
                    lines.append(f"    \"{src}\"")
                lines.append(")")
            else:
                # Create dummy source if no sources found
                lines.append(f"# No sources found, creating dummy source")
                lines.append(f"file(WRITE ${{CMAKE_BINARY_DIR}}/dummy_{name}.cpp \"int main() {{ return 0; }}\")")
                win32_flag = "WIN32 " if is_win32 else ""
                lines.append(f"add_executable({name} {win32_flag}${{CMAKE_BINARY_DIR}}/dummy_{name}.cpp)")
            
            lines.append("")
            
            # Add JUCE-specific settings if it's a JUCE project
            if is_juce:
                lines.append(f"# JUCE modules and settings for {name}")
                
                # Include directories
                lines.append(f"target_include_directories({name} PRIVATE")
                if juce_info.get("use_modern_cmake"):
                    lines.append("    # Include paths handled by JUCE CMake")
                else:
                    lines.append("    ${JUCE_MODULE_PATH}")
                lines.append(f"    \"${{CMAKE_CURRENT_SOURCE_DIR}}/JuceLibraryCode\"")
                lines.append(f"    \"${{CMAKE_CURRENT_SOURCE_DIR}}/Source\"")
                lines.append(")")
                lines.append("")
                
                # Add detected JUCE module definitions
                if juce_info.get("modules"):
                    lines.append(f"# Detected JUCE modules")
                    lines.append(f"target_compile_definitions({name} PRIVATE")
                    lines.append("    JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1")
                    lines.append("    JUCE_STANDALONE_APPLICATION=1")
                    for module in juce_info["modules"]:
                        module_upper = module.upper().replace("-", "_")
                        lines.append(f"    JUCE_MODULE_AVAILABLE_{module}=1")
                    lines.append(")")
                    lines.append("")
                
                # Add Windows system libraries for JUCE
                if is_win32:
                    lines.append(f"# Windows system libraries required by JUCE")
                    lines.append(f"target_link_libraries({name} PRIVATE")
                    for lib in JUCE_WINDOWS_LIBS:
                        lines.append(f"    {lib}")
                    lines.append(")")
                    lines.append("")
        
        # Add configuration-specific settings (for traditional approach)
        if not (is_juce and juce_info.get("use_modern_cmake")):
            for config_name, cfg in configs.items():
                if any([cfg["include_dirs"], cfg["defines"], cfg["lib_dirs"], 
                       cfg["libs"], cfg["compile_options"]]):
                    lines.append(f"# {config_name} configuration")
                
                # Include directories
                if cfg["include_dirs"]:
                    # Filter out directories already added for JUCE
                    dirs_to_add = cfg["include_dirs"]
                    if is_juce:
                        # Skip JUCE module path and JuceLibraryCode as they're already added
                        dirs_to_add = [d for d in dirs_to_add 
                                     if "JUCE" not in d or "modules" not in d]
                        dirs_to_add = [d for d in dirs_to_add if "JuceLibraryCode" not in d]
                    
                    if dirs_to_add:
                        lines.append(f"target_include_directories({name} PRIVATE")
                        lines.append(f"    $<$<CONFIG:{config_name}>:")
                        for dir in dirs_to_add:
                            lines.append(f"        \"{dir}\"")
                        lines.append("    >")
                        lines.append(")")
                
                # Compile definitions
                if cfg["defines"]:
                    lines.append(f"target_compile_definitions({name} PRIVATE")
                    lines.append(f"    $<$<CONFIG:{config_name}>:")
                    for define in cfg["defines"]:
                        # Escape equals signs in definitions
                        define_escaped = define.replace("=", "\\=")
                        lines.append(f"        {define_escaped}")
                    lines.append("    >")
                    lines.append(")")
                
                # Library directories
                if cfg["lib_dirs"]:
                    lines.append(f"target_link_directories({name} PRIVATE")
                    lines.append(f"    $<$<CONFIG:{config_name}>:")
                    for dir in cfg["lib_dirs"]:
                        lines.append(f"        \"{dir}\"")
                    lines.append("    >")
                    lines.append(")")
                
                # Libraries (filter out Windows system libs if already added for JUCE)
                libs_to_add = cfg["libs"]
                if is_juce and is_win32:
                    libs_to_add = [lib for lib in libs_to_add if lib not in JUCE_WINDOWS_LIBS]
                
                if libs_to_add:
                    lines.append(f"target_link_libraries({name} PRIVATE")
                    lines.append(f"    $<$<CONFIG:{config_name}>:")
                    for lib in libs_to_add:
                        lines.append(f"        {lib}")
                    lines.append("    >")
                    lines.append(")")
                
                # Compile options
                compile_opts = cfg["compile_options"].copy()
                
                # For JUCE projects, ensure /bigobj is included
                if is_juce and "/bigobj" not in compile_opts:
                    compile_opts.append("/bigobj")
                
                if compile_opts:
                    lines.append(f"target_compile_options({name} PRIVATE")
                    lines.append(f"    $<$<CONFIG:{config_name}>:")
                    for opt in compile_opts:
                        lines.append(f"        {opt}")
                    lines.append("    >")
                    lines.append(")")
                
                lines.append("")
        
        # Add optimization flags
        if is_juce:
            lines.append("# Optimization settings")
            lines.append("if(MSVC)")
            lines.append(f"    target_compile_options({name} PRIVATE")
            lines.append("        $<$<CONFIG:Debug>:/Od /RTC1 /GS>")
            lines.append("        $<$<CONFIG:Release>:/O2 /Ob2 /GF /Gy>")
            lines.append("    )")
            lines.append(f"    target_link_options({name} PRIVATE")
            lines.append("        $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>")
            lines.append("        $<$<CONFIG:Release>:/OPT:REF /OPT:ICF /INCREMENTAL:NO>")
            lines.append("    )")
            lines.append("endif()")
            lines.append("")
        
        lines.append("")
    
    # Export target names for build script
    lines.append("# Export target names for build script")
    lines.append("file(WRITE ${CMAKE_BINARY_DIR}/targets.txt \"" + "\\n".join(target_names) + "\")")
    lines.append("")
    
    # Print summary
    if has_juce:
        lines.append("# Configuration summary")
        lines.append("message(STATUS \"===========================================\")")
        lines.append("message(STATUS \"JUCE CMake Configuration Complete\")")
        lines.append("message(STATUS \"===========================================\")")
        lines.append("message(STATUS \"Targets: " + ", ".join(target_names) + "\")")
        lines.append("message(STATUS \"Build Type: ${CMAKE_BUILD_TYPE}\")")
        if juce_config and juce_config.get("juce_modules_path"):
            lines.append("message(STATUS \"JUCE Modules: ${JUCE_MODULE_PATH}\")")
        lines.append("message(STATUS \"===========================================\")")
    
    return "\n".join(lines)

def main():
    """Main entry point."""
    
    print("="*60)
    print("VS2022 to CMake Converter (JUCE-aware)")
    print("="*60)
    print()
    
    # Check and initialize JUCE submodule if needed
    if not check_and_init_juce_submodule():
        print("[ERROR] Failed to initialize JUCE submodule. Continuing anyway...")
    print()
    
    # Find all .vcxproj files (only in Builds/VisualStudio2022)
    root = Path.cwd()
    builds_dir = root / "Builds" / "VisualStudio2022"
    if builds_dir.exists():
        vcxproj_files = list(builds_dir.glob("*.vcxproj"))
    else:
        # Fallback to all vcxproj files in current directory
        vcxproj_files = [f for f in root.glob("*.vcxproj") if "JUCE" not in str(f)]
    
    if not vcxproj_files:
        print("[ERROR] No .vcxproj files found in current directory or subdirectories.")
        return 1
    
    print(f"[INFO] Found {len(vcxproj_files)} .vcxproj file(s)")
    
    # Parse all projects
    projects = []
    for vcxproj in vcxproj_files:
        # Skip backup, temporary, and CMake-generated files
        skip_patterns = ["backup", "temp", "old", ".user", "cmake", "CMakeFiles", 
                        "CompilerId", "VCTargetsPath"]
        if any(x in vcxproj.name.lower() for x in skip_patterns):
            print(f"[SKIP] Ignoring {vcxproj.name}")
            continue
        
        proj_data = parse_vcxproj(vcxproj)
        if proj_data:
            projects.append(proj_data)
    
    if not projects:
        print("[ERROR] No valid projects could be parsed.")
        return 2
    
    # Generate CMakeLists.txt
    print()
    print("[INFO] Generating CMakeLists.txt...")
    cmake_content = generate_cmakelists(projects)
    
    # Write to file (always delete old CMakeLists.txt first)
    output_path = root / "CMakeLists.txt"
    try:
        if output_path.exists():
            print(f"[INFO] Removing existing {output_path}")
            output_path.unlink()
    except Exception as e:
        print(f"[WARN] Failed to remove existing {output_path}: {e}")
    output_path.write_text(cmake_content, encoding="utf-8")
    
    print(f"[SUCCESS] Generated {output_path}")
    print()
    print("[INFO] Detected targets:")
    for proj in projects:
        original_name = proj.get('original_name', proj['name'])
        cmake_name = proj['name']
        juce_tag = " (JUCE)" if proj.get("is_juce") else ""
        
        if original_name != cmake_name:
            print(f"  - {original_name} -> {cmake_name}{juce_tag}")
        else:
            print(f"  - {cmake_name}{juce_tag}")
            
        if proj.get("juce_info", {}).get("modules"):
            print(f"    Modules: {', '.join(proj['juce_info']['modules'])}")
    
    print()
    print("[INFO] Next step: Run 'build_and_run.bat' or 'build_and_run.bat rebuild'")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())