"""Keep Java and C# API signatures readable in Breathe."""

from breathe.renderer.sphinxrenderer import BaseObject, DomainDirectiveFactory
from sphinx import addnodes
from sphinx.directives import ObjectDescription


class JavaSignature(BaseObject, ObjectDescription):
    def handle_signature(self, signature, signode):
        signature = signature.replace("::", ".").strip()
        if self.objtype == "class":
            # Breathe uses C++ base class syntax.
            signature = signature.replace(" : public ", " extends ")
        if self.objtype in {"namespace", "class", "interface", "struct", "enum"}:
            annotation = f"{self.objtype} "
            signode += addnodes.desc_annotation(annotation, annotation)
        signode += addnodes.desc_name(signature, signature)
        return signature


def setup(app):
    # Breathe has no Java domain. Preserve its descriptions and Doxygen
    # anchors while rendering Java signatures without the C++ parser.
    original_create = DomainDirectiveFactory.create

    def create(domain, args):
        kind = args[0]
        if domain in {"java", "cs"} and kind in {"class", "interface", "struct"}:
            # Breathe prepends the namespace to an already qualified name.
            signatures = [sig.replace("brainflow.brainflow.", "brainflow.") for sig in args[1]]
            args = [kind, signatures, *args[2:]]
        if domain == "java":
            return JavaSignature(f"java:{kind}", *args[1:])
        return original_create(domain, args)

    DomainDirectiveFactory.create = staticmethod(create)
    return {"parallel_read_safe": True, "parallel_write_safe": True}
