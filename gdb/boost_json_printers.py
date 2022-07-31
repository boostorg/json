import gdb
import gdb.printing


class PrettyPrinterBuilder():
    """decorator that accumulates pretty printers for types"""

    def __init__(self, name=None):
        self.result = gdb.printing.RegexpCollectionPrettyPrinter(
            name or "Boost.JSON")

    def __call__(self, pp=None, ns=None, template=False):
        if pp is None:
            def decorator(pp):
                return self(pp, ns=ns, template=template)
            return decorator

        typename = getattr(pp, '__name__')
        ns = ns or "boost::json"
        self.result.add_printer(
            typename,
            "^{ns}::{typename}{marker}".format(
                ns=ns,
                typename=typename,
                marker="<"if template else "$"),
            pp)
        return pp

pretty_printer = PrettyPrinterBuilder()


class static_property:
    """decorator for lazy evaluation of static members"""

    def __init__(self, wrapped):
        self.wrapped = wrapped
        self.__name__ = wrapped.__name__
        self.__doc__ = wrapped.__doc__

    def __get__(self, inst, objtype=None):
        val = self.wrapped()
        if objtype is not None:
            setattr(objtype, self.wrapped.__name__, val)
        return val


@pretty_printer(ns="boost::json::detail", template=True)
class shared_resource_impl:
    """boost::json::detail::shared_resource_impl<T> pretty printer"""

    def __init__(self, val):
        self.val = val["t"]

    def to_string(self):
        return self.val


@pretty_printer(ns="boost::json::detail")
class shared_resource:
    """boost::json::detail::shared_resource pretty printer"""

    @static_property
    def size_t():
        return gdb.lookup_type("std::size_t")

    @staticmethod
    def get_items(value):
        impl_ptr = value.dynamic_type.pointer()
        resource = value.address.cast(impl_ptr).dereference()
        return [
            ("refs", value["refs"].cast(shared_resource.size_t)),
            ("resource", resource),
        ]

    def __init__(self, val):
        self.val = val

    def to_string(self):
        items = [
            ("%s=%s" % item)
            for item in self.get_items(self.val)
        ]
        return "shared_resource [" + ", ".join(items) + "]"


@pretty_printer
class storage_ptr:
    """boost::json::storage_ptr pretty printer"""

    @static_property
    def shared_resource_ptr():
        return gdb.lookup_type(
            "boost::json::detail::shared_resource").pointer()

    @static_property
    def memory_resource_ptr():
        return gdb.lookup_type("boost::json::memory_resource").pointer()

    def __init__(self, val):
        self.val = val["i_"]

    def to_string(self):
        items = []

        shared = bool(self.val & 1)
        trivial = bool(self.val & 2)
        if shared:
            items.append("shared")
        if trivial:
            items.append("trivial")

        pointer = self.val & ~3
        if not pointer:
            items.append("resource=default")
        else:
            if shared:
                resource = pointer.cast(self.shared_resource_ptr).dereference()
                for item in shared_resource.get_items(resource):
                    items.append("%s=%s" % item)
            else:
                resource = pointer.cast(self.memory_resource_ptr).dereference()
                items.append("resource=%s" % resource)

        return "storage_ptr [" + ", ".join(items) + "]"


@pretty_printer(ns="boost::container::pmr")
class memory_resource:
    """boost::json::memory_resource pretty printer"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        derived_t = self.val.dynamic_type
        if derived_t.code == gdb.TYPE_CODE_REF:
            derived_t = derived_t.target()
        derived_ptr = derived_t.pointer()
        return self.val.address.cast(derived_ptr).dereference()


@pretty_printer
class monotonic_resource:
    """boost::json::monotonic_resource pretty printer"""

    def __init__(self, val):
        self.val = val["head_"].dereference()

    def to_string(self):
        return "monotonic_resource [buffer=%s, free=%s]" % (
            self.val["p"], self.val["avail"])


@pretty_printer
class static_resource:
    """boost::json::static_resource pretty printer"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "static_resource [buffer=%s, free=%s]" % (
            self.val["p_"], self.val["n_"])


@pretty_printer
class string:
    """boost::json::string pretty printer"""

    @static_property
    def char_const_ptr():
        return gdb.lookup_type("char").const().pointer()

    def __init__(self, val):
        self.impl = val["impl_"]

    def to_string(self):
        kind = self.impl["s_"]["k"]
        if kind == self.impl["short_string_"]:
            sbo_size = self.impl["sbo_chars_"]
            size = sbo_size - self.impl["s_"]["buf"][sbo_size]
            pointer = self.impl["s_"]["buf"]
        elif kind == self.impl["key_string_"]:
            size = self.impl["k_"]["n"]
            pointer = self.impl["k_"]["s"]
        else:
            size = self.impl["p_"]["t"].dereference()["size"]
            pointer = self.impl["p_"]["t"]
            pointer += 1
            pointer = pointer.cast(self.char_const_ptr)
        return pointer.lazy_string(length=size)

    def display_hint(self):
        return "string"


@pretty_printer
class array:
    """boost::json::array pretty printer"""

    @static_property
    def value_ptr():
        return gdb.lookup_type("boost::json::value").pointer()

    def __init__(self, val):
        self.val = val

    def to_string(self):
        capacity = int(self.val["t_"].dereference()["capacity"])
        return "array [size={}, capacity={}]".format(self._size(), capacity)

    def children(self):
        table = (self.val["t_"] + 1).cast(self.value_ptr)
        for i in range(0, self._size()):
            yield str(i), table[i]

    def display_hint(self):
        return "array"

    def _size(self):
        return int(self.val["t_"].dereference()["size"])


@pretty_printer
class key_value_pair:
    """boost::json::key_value_pair pretty printer"""

    @staticmethod
    def pair(kv):
        return kv["key_"].lazy_string(length=kv["len_"]), kv["value_"]

    def __init__(self, val):
        self.val = val

    def to_string(self):
        k, v = self.pair(self.val)
        return "[%s] = %s" % (k.value(), v.format_string())


@pretty_printer
class object:
    """boost::json::object pretty printer"""

    def __init__(self, val):
        self.val = val
        self.kv_ptr = gdb.lookup_type("boost::json::key_value_pair").pointer()

    def to_string(self):
        capacity = int(self.val["t_"].dereference()["capacity"])
        return "object [size={}, capacity={}]".format(self._size(), capacity)

    def children(self):
        table = (self.val["t_"] + 1).cast(self.kv_ptr)
        for i in range(0, self._size()):
            k, v = key_value_pair.pair(table[i])
            yield str(2 * i), k
            yield str(2 * i + 1), v

    def display_hint(self):
        return "map"

    def _size(self):
        return int(self.val["t_"].dereference()["size"])


@pretty_printer
class value:
    """boost::json::value pretty printer"""

    @static_property
    def kind_t():
        result = gdb.lookup_type("boost::json::kind")
        return dict(result.items())

    def __init__(self, val):
        self.val = val

    def to_string(self):
        kind = self.val["sca_"]["k"]

        if self._compare_kind(kind, "null"):
            return "null"

        elif self._compare_kind(kind, "bool_"):
            return self.val["sca_"]["b"]

        elif self._compare_kind(kind, "int64"):
            return self.val["sca_"]["i"]

        elif self._compare_kind(kind, "uint64"):
            return self.val["sca_"]["u"]

        elif self._compare_kind(kind, "double_"):
            return self.val["sca_"]["d"]

        elif self._compare_kind(kind, "array"):
            return self.val["arr_"]

        elif self._compare_kind(kind, "object"):
            return self.val["obj_"]

        else:
            return self.val["str_"]

    def display_hint(self):
        return "boost::json::value"

    def _compare_kind(self, kind, name):
        return kind == self.kind_t["boost::json::kind::" + name].enumval


def register(obj_file):
    mod = obj_file or gdb
    for printer in getattr(mod, "pretty_printers", []):
        if getattr(printer, "name") == pretty_printer.result.name:
            return

    gdb.printing.register_pretty_printer(
        obj_file,
        pretty_printer.result)


if __name__ == '__main__':
    register(gdb.current_objfile())
