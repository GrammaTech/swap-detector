import os
import lit
from lit.llvm import llvm_config

config.name = "SwappedArgs"
config.test_source_root = os.path.dirname(__file__)
config.suffixes = [".c", ".cpp"]
config.test_format = lit.formats.ShTest(not llvm_config.use_lit_shell)
llvm_config.use_default_substitutions()
llvm_config.use_clang()

if sys.platform in ['win32', 'cygwin']:
    has_plugins = config.enable_shared
else:
    has_plugins = True
if has_plugins and config.llvm_plugin_ext:
    config.available_features.add('plugins')

config.substitutions.append(('%shlibext', config.llvm_shlib_ext))

# %clang_analyze_cc1's expansion includes %analyze, which lets CSA's tests work
# against the range constraint manager or Z3. Avoid that complexity for now.
config.substitutions.append(('%analyze', '-analyzer-constraints=range'))

if lit_config.params.get('DUMP_CONFIG'):
    from pprint import *
    pprint(config.__dict__)
