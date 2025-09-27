# dlpi_promiscon

`dlpi_promiscon` is a utility for Solaris/Illumos systems that sets a network interface into **promiscuous mode**.  
In promiscuous mode, the interface receives all packets on the network segment, not just those addressed to it. This is useful for packet capture and network monitoring.

## Build

This program requires `libdlpi` in your development environment.

```sh
cc -o dlpi_promiscon dlpi_promiscon.c -ldlpi
```

## Usage

```sh
./dlpi_promiscon <network-interface>
```

Example: enable promiscuous mode on `net0`

```sh
./dlpi_promiscon net0
```

## Use Case: Nested Virtualization

When you create additional virtual NICs behind a virtual switch or bridge, the parent interface usually only accepts frames destined to its own MAC address.
As a result, frames for downstream virtual NICs may not be delivered correctly.

Enabling promiscuous mode on the parent interface ensures that it receives all frames, allowing them to be passed down to the nested virtual NICs or VNICs.

## Notes

- Must be run with **root privileges**.
- Works only on Solaris/Illumos; it will not run on Linux/BSD.
- Promiscuous mode can pose security risks—use it only when necessary.

## License

* CDDL-1.0
* https://opensource.org/license/CDDL-1.0