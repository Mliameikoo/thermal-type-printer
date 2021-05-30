# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.

import os

target_extension = '.uvprojx'
# target search-based param: <IncludePath>, <Define>


class FileSearch:
    def __init__(self):
        self.target_filename = None
        self.is_searching_valid = False
        self.read_include_path_list = None
        self.read_define_param_list = None

    def find_project_file(self):
        path = os.getcwd()
        for root, dirs, files in os.walk(path):
            for file in files:
                # divide file as <name,extension>
                (file_name, file_extension) = os.path.splitext(file)
                if file_extension == target_extension:
                    self.target_filename = os.path.join(root, file_name)
                    self.is_searching_valid = True

    def export_project_name(self):
        if not self.is_searching_valid:
            print(" * find project file failure")
        else:
            print(" * find project file success: " + self.target_filename + target_extension)

    def export_include_path(self):
        file_handle = self.target_filename + target_extension
        with open(file_handle, 'r') as file:
            read_contents = file.readlines()
            for read_content in read_contents:
                if '<IncludePath>' in read_content:
                    read_content = read_content.replace('<IncludePath>', '')
                    read_content = read_content.replace('</IncludePath>', '')
                    read_content = read_content.replace('\n', '')
                    read_content = read_content.replace(' ', '')
                    read_content = read_content.replace('..', '${workspaceFolder}')
                    read_content = read_content.replace('\\', '/')
                    read_include_path_list = read_content.split(';')
                    if len(read_include_path_list) - 1 > 0:
                        self.read_include_path_list = read_include_path_list
                        print(" * ready to write ", end="")
                        print(self.read_include_path_list)

    def export_path_to_file(self):
        # write new output-file
        with open('IncludePath.txt', 'w') as file:
            if len(self.read_include_path_list) - 1 > 0:
                file.writelines('<IncludePath>' + '\n')
                for write_content in self.read_include_path_list:
                    file.writelines('"' + write_content + '"' + ',' + '\n')
                print(" * extract <IncludePath> success as <IncludePath.txt>")
            else:
                print(" * can't find <IncludePath> param")

    def export_define_param(self):
        file_handle = self.target_filename + target_extension
        with open(file_handle, 'r') as file:
            read_contents = file.readlines()
            for read_content in read_contents:
                if '<Define>' in read_content:
                    read_content = read_content.replace('<Define>', '')
                    read_content = read_content.replace('</Define>', '')
                    read_content = read_content.replace('\n', '')
                    read_content = read_content.replace(' ', '')
                    read_define_param_list = read_content.split(',')
                    if len(read_define_param_list) - 1 > 0:
                        self.read_define_param_list = read_define_param_list
                        print(" * ready to write ", end="")
                        print(self.read_define_param_list)

    def export_define_param_to_file(self):
        # add define param to output-txt
        with open('IncludePath.txt', 'a') as file:
            if len(self.read_define_param_list) - 1 > 0:
                file.writelines('<Define>' + '\n')
                for write_content in self.read_define_param_list:
                    file.writelines('"' + write_content + '"' + ',' + '\n')
                # add additional define <__CC_ARM> to fix missing of "uint32_t"
                file.writelines('"__CC_ARM"' + '\n')
                print(" * extract <Define> success add in <IncludePath.txt>")
            else:
                print(" * can't find <Define> param")


# Press the green button in the gutter to run the script.
# if __name__ == '__main__':
print("")
print("******************tool-begin****************")
fileSearch = FileSearch()
fileSearch.find_project_file()
fileSearch.export_project_name()
fileSearch.export_include_path()
fileSearch.export_path_to_file()
fileSearch.export_define_param()
fileSearch.export_define_param_to_file()
print("******************tool-exit*****************")
print("")

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
