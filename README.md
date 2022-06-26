
# Student management driver linux



## Run Locally

Clone the project

```bash
  git clone https://github.com/ncuy0110/student-management-driver-linux.git
```

Go to the project directory

```bash
  cd student-management-driver-linux
```

Compile driver

```bash
  make
```

Insert to kernel

```bash
    sudo insmod mydriver2.ko
```

Access device

```bash
    sudo chmod 666 /dev/vchar_dev
```

go to user app

```bash
    cd user_app
```

Compile user app

```bash
    make
```

Run user app

```bash
    ./user_app
```