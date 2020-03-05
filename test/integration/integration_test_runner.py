import argparse
import io
import json
import subprocess

parser = argparse.ArgumentParser(
    description="Runs the swapped argument integration tests."
)
parser.add_argument("--input", type=argparse.FileType("r"), help="The name database to load.", default="names_subset.json")
parser.add_argument("--executable", help="The test executable to run.", default="IntegrationTestSwappedArgs")
args = parser.parse_args()

with args.input as f:
  for line in f:
    doc = json.loads(line)

    for fname, func in doc['functions'].items():
      decl_attrs = func.get("declAttrs", {})
      param_names = None
      call_decl_line = None
      call_decl_file = None

      if "location" in decl_attrs:
        call_decl_line = decl_attrs["location"].get("lineNo")
        call_decl_file = doc['fileNameMap'][decl_attrs["location"].get("file")]

      if "params" in decl_attrs:
        param_names = []
        for param in decl_attrs["params"]:
          param_names.append(param.get("name", ""))

        for site in func['callSites']:
          arg_names = [arg.get("name", "") for arg in site['attrs']['args']]
          proc_args = [args.executable, '--function', fname, '--args'] + arg_names
          if param_names is not None:
            proc_args.append('--params')
            proc_args += param_names
          call_site_line = site['site'].get("lineNo", 0)
          call_site_file = doc['fileNameMap'][site['site'].get("file")]
          if call_decl_line is not None and call_decl_file is not None:
            proc_args.extend(['--call_decl_file', call_decl_file, '--call_decl_line', str(call_decl_line)])
          proc_args.extend(['--call_site_file', call_site_file, '--call_site_line', str(call_site_line)])
          subprocess.call(proc_args)
