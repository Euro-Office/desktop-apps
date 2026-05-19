---
id: 5-5
title: Avoid innerHTML
category: Refs & DOM
priority: HIGH
description: Avoid innerHTML to prevent XSS vulnerabilities — use JSX or textContent instead
---

## Problem

Solid.js exposes `innerHTML` as a JSX prop for convenience, but using it introduces **cross-site scripting (XSS)** vulnerabilities. Any unsanitized user input injected via `innerHTML` can execute arbitrary JavaScript. In most cases, the same result can be achieved safely with JSX or `textContent`.

## Incorrect

```tsx
// ❌ WRONG: User input directly in innerHTML — XSS vulnerability
function Comment(props) {
  return <div innerHTML={props.commentHtml} />;
}

// An attacker could submit:
// <img src=x onerror="document.cookie='stolen'">
```

```tsx
// ❌ WRONG: Building HTML strings with interpolation
function UserBio(props) {
  const html = `<h2>${props.user.name}</h2><p>${props.user.bio}</p>`;
  return <div innerHTML={html} />;
}
```

```tsx
// ❌ WRONG: Assuming server data is safe
function Article(props) {
  const [article] = createResource(() => fetchArticle(props.id));

  return (
    <Show when={article()}>
      <div innerHTML={article().body} />
    </Show>
  );
}
```

## Correct

### Use JSX Instead

```tsx
// ✅ CORRECT: JSX auto-escapes text content
function Comment(props) {
  return (
    <div>
      <p>{props.commentText}</p>
    </div>
  );
}
```

```tsx
// ✅ CORRECT: Build UI with JSX, not HTML strings
function UserBio(props) {
  return (
    <div>
      <h2>{props.user.name}</h2>
      <p>{props.user.bio}</p>
    </div>
  );
}
```

### When HTML Rendering Is Required

If you genuinely need to render HTML (e.g., from a CMS or Markdown renderer), sanitize it first:

```tsx
import DOMPurify from "dompurify";

// ✅ CORRECT: Sanitize before rendering
function Article(props) {
  const [article] = createResource(() => fetchArticle(props.id));

  const sanitizedBody = createMemo(() => {
    const raw = article()?.body;
    return raw ? DOMPurify.sanitize(raw) : "";
  });

  return (
    <Show when={article()}>
      <div innerHTML={sanitizedBody()} />
    </Show>
  );
}
```

### Use textContent for Plain Text

```tsx
// ✅ CORRECT: textContent is safe for plain text
function Badge(props) {
  return <span textContent={props.label} />;
}
```

### Wrap in a Sanitizing Component

```tsx
// ✅ CORRECT: Reusable safe HTML component
import DOMPurify from "dompurify";

function SafeHtml(props: { html: string; class?: string }) {
  const clean = createMemo(() => DOMPurify.sanitize(props.html));

  return <div class={props.class} innerHTML={clean()} />;
}

// Usage
<SafeHtml html={article().body} class="prose" />
```

## When innerHTML Is Acceptable

| Scenario | Safe? | Notes |
| -------- | ----- | ----- |
| User-generated content | No | Always sanitize |
| CMS/Markdown output | Sanitize first | Use DOMPurify or similar |
| Static HTML from build tools | Generally safe | Content is trusted |
| SVG icons from trusted source | Generally safe | No user input involved |
| Any string with interpolation | No | Injection risk |

## Why It Matters

1. **Security**: XSS is a critical vulnerability. Injected scripts can steal cookies, redirect users, or perform actions on their behalf.

2. **Solid-Specific Risk**: Unlike React (which escapes by default and requires `dangerouslySetInnerHTML`), Solid provides `innerHTML` as a regular prop with no warning name. It's easier to use accidentally.

3. **Defense in Depth**: Even if you trust your data source, sanitizing HTML provides a safety net against upstream compromises.

4. **OWASP Top 10**: Cross-site scripting is consistently in the OWASP Top 10 web application security risks.

## Related Rules

- [5-1: Use Refs Correctly](5-1-use-refs-correctly.md) - Safe DOM access patterns
- [5-2: Access DOM in onMount](5-2-access-dom-in-onmount.md) - Proper DOM manipulation timing
