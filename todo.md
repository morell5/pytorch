1. Find the reason of the problem

    ```
        o CUDA runtime is found, using CUDA_HOME='/usr/local/cuda'
    Traceback (most recent call last):
    File "test_cpp_api_parity.py", line 55, in <module>
        module_impl_check.build_cpp_tests(TestCppApiParity, print_cpp_source=PRINT_CPP_SOURCE)
    File "/home/morell/projects/pytorch/test/cpp_api_parity/module_impl_check.py", line 297, in build_cpp_tests
        functions=functions)
    File "/home/morell/projects/pytorch/test/cpp_api_parity/utils.py", line 148, in compile_cpp_code_inline
        verbose=False,
    File "/home/morell/projects/pytorch/env/lib/python3.6/site-packages/torch/utils/cpp_extension.py", line 1158, in load_inline
        keep_intermediates=keep_intermediates)
    File "/home/morell/projects/pytorch/env/lib/python3.6/site-packages/torch/utils/cpp_extension.py", line 1213, in _jit_compile
        with_cuda=with_cuda)
    File "/home/morell/projects/pytorch/env/lib/python3.6/site-packages/torch/utils/cpp_extension.py", line 1279, in _write_ninja_file_and_build_library
        verify_ninja_availability()
    File "/home/morell/projects/pytorch/env/lib/python3.6/site-packages/torch/utils/cpp_extension.py", line 1334, in verify_ninja_availability
        raise RuntimeError("Ninja is required to load C++ extensions")
    RuntimeError: Ninja is required to load C++ extensions
    Traceback (most recent call last):
    File "/home/morell/projects/pytorch/test/run_test.py", line 873, in <module>
        main()
    File "/home/morell/projects/pytorch/test/run_test.py", line 856, in main
        raise RuntimeError(err_message)
    RuntimeError: test_cpp_api_parity failed!
    ```

1. Переустановить Anaconda

    guide: https://docs.anaconda.com/anaconda/install/linux/
    archive: https://repo.anaconda.com/archive/

    installer: projects: anaconda3 4.3.0

    Сделать, чтобы по python3 запускался python3.6

    Сейчас: запуск
    ```
    /home/morell/.local/bin/conda -v
    ```

    Приводит к:
    ```
    bash: /home/morell/.local/bin/conda: /usr/bin/python3: bad interpreter: No such file or directory
    ```
    
    Решение: дать reference на python3

    ```
    sudo ln -s /usr/bin/python3.6 /usr/bin/python3
    ```

    Знать различия в anaconda2, anaconda3

2. Ошибка:

    ```
    conda no module cli
    ```

    Решение: добавить conda в PATH
    ```
    export PATH=/home/morell/anaconda3/bin:$PATH
    ```

3. Установить packages c pytorch github

    ```
    conda install numpy 
    conda install ninja pyyaml mkl mkl-include setuptools cmake cffi
    #TODO:
    conda install typing_extensions future six requests dataclasses
    ```

    ```
    conda install -c pytorch magma-cuda102  # or [ magma-cuda101 | magma-cuda100 | magma-cuda92 ] depending on your cuda version
    ```
4. Сделать: Удалить все версии встроенного python
5. Сделать: Найти способ фиксации


    ```
    export PATH=/home/morell/anaconda3/bin:$PATH
    ```
6. Переустановить torch через setup.py

7. Установить причину и устранить

    ```
    import torch
    Traceback (most recent call last):
    File "<stdin>", line 1, in <module>
    File "/home/morell/projects/pytorch/torch/__init__.py", line 201, in <module>
        from torch._C import _initExtension
    ImportError: cannot import name '_initExtension'
    ```

    Причина:

    ```
    В директории _C файл __init__.pyi, а не __init__.py. В __init__.pyi есть объявление _initExtension
    ```

    Сделать:
    * выяснить назначение расширения *.pyi
    * выяснить должен ли быть файл __init__.py в _C

8. Установить какая версия hypothesis совместима с текущей версией pytorch. Установить hypothesis