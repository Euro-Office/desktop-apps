---
id: 7-1
title: Use Semantic HTML
category: Accessibility
priority: HIGH
description: Use appropriate semantic HTML elements
---

## Problem

Using generic elements (`<div>`, `<span>`) where semantic elements exist harms accessibility. Screen readers, search engines, and assistive technologies rely on proper HTML semantics to understand and navigate content.

## Incorrect

```tsx
// ❌ WRONG: Generic elements for semantic content
function ArticlePage() {
  return (
    <div class="page">
      <div class="header">
        <div class="logo">Site Name</div>
        <div class="nav">
          <div onClick={...}>Home</div>
          <div onClick={...}>About</div>
        </div>
      </div>

      <div class="main">
        <div class="article">
          <div class="title">Article Title</div>
          <div class="content">
            <div class="paragraph">First paragraph...</div>
            <div class="paragraph">Second paragraph...</div>
          </div>
        </div>
      </div>

      <div class="footer">
        <div>© 2024</div>
      </div>
    </div>
  );
}
```

```tsx
// ❌ WRONG: Div as button
function ClickableCard(props) {
  return (
    <div class="card" onClick={props.onClick}>
      <div class="card-title">{props.title}</div>
    </div>
  );
}
```

## Correct

```tsx
// ✅ CORRECT: Semantic HTML elements
function ArticlePage() {
  return (
    <div class="page">
      <header>
        <h1 class="logo">Site Name</h1>
        <nav>
          <a href="/">Home</a>
          <a href="/about">About</a>
        </nav>
      </header>

      <main>
        <article>
          <h2>Article Title</h2>
          <p>First paragraph...</p>
          <p>Second paragraph...</p>
        </article>
      </main>

      <footer>
        <p>© 2024</p>
      </footer>
    </div>
  );
}
```

```tsx
// ✅ CORRECT: Button element for clickable actions
function ClickableCard(props) {
  return (
    <article class="card">
      <h3 class="card-title">{props.title}</h3>
      <button onClick={props.onClick}>View Details</button>
    </article>
  );
}

// Or if the whole card should be clickable:
function ClickableCard(props) {
  return (
    <article class="card">
      <a href={props.href} class="card-link">
        <h3>{props.title}</h3>
        <p>{props.description}</p>
      </a>
    </article>
  );
}
```

### Lists

```tsx
// ❌ WRONG
<div class="list">
  <div class="item">Item 1</div>
  <div class="item">Item 2</div>
</div>

// ✅ CORRECT
<ul>
  <li>Item 1</li>
  <li>Item 2</li>
</ul>

// ✅ CORRECT: Ordered list for sequences
<ol>
  <li>First step</li>
  <li>Second step</li>
</ol>

// ✅ CORRECT: Description list for key-value pairs
<dl>
  <dt>Name</dt>
  <dd>John Doe</dd>
  <dt>Email</dt>
  <dd>john@example.com</dd>
</dl>
```

### Forms

```tsx
// ❌ WRONG: Missing labels
<div class="form-group">
  <span>Email</span>
  <input type="email" />
</div>

// ✅ CORRECT: Proper form semantics
<form onSubmit={handleSubmit}>
  <fieldset>
    <legend>Contact Information</legend>

    <div class="form-group">
      <label for="email">Email</label>
      <input type="email" id="email" name="email" required />
    </div>

    <div class="form-group">
      <label for="message">Message</label>
      <textarea id="message" name="message" required />
    </div>
  </fieldset>

  <button type="submit">Send</button>
</form>
```

### Tables

```tsx
// ❌ WRONG: Div-based table
<div class="table">
  <div class="row header">
    <div class="cell">Name</div>
    <div class="cell">Email</div>
  </div>
  <div class="row">
    <div class="cell">John</div>
    <div class="cell">john@example.com</div>
  </div>
</div>

// ✅ CORRECT: Semantic table
<table>
  <thead>
    <tr>
      <th scope="col">Name</th>
      <th scope="col">Email</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>John</td>
      <td>john@example.com</td>
    </tr>
  </tbody>
</table>
```

## Semantic Element Reference

| Content Type | Use Element                  |
| ------------ | ---------------------------- |
| Page header  | `<header>`                   |
| Navigation   | `<nav>`                      |
| Main content | `<main>`                     |
| Article/post | `<article>`                  |
| Sidebar      | `<aside>`                    |
| Page footer  | `<footer>`                   |
| Section      | `<section>` (with heading)   |
| Headings     | `<h1>` - `<h6>` (in order)   |
| Paragraphs   | `<p>`                        |
| Lists        | `<ul>`, `<ol>`, `<dl>`       |
| Interactive  | `<button>`, `<a>`, `<input>` |
| Time/dates   | `<time datetime="...">`      |
| Figures      | `<figure>` + `<figcaption>`  |
| Code         | `<code>`, `<pre>`            |
| Quotes       | `<blockquote>`, `<q>`        |
| Emphasis     | `<strong>`, `<em>`           |

## Why It Matters

1. **Screen Readers**: Announce elements by type, enabling efficient navigation.

2. **Keyboard Navigation**: Native elements have built-in keyboard support.

3. **SEO**: Search engines understand semantic content better.

4. **Maintainability**: Semantic code is self-documenting.

5. **Future-Proof**: Works with new assistive technologies automatically.

## Heading Hierarchy

```tsx
// ✅ CORRECT: Logical heading order
<main>
  <h1>Page Title</h1>

  <section>
    <h2>Section Title</h2>
    <p>Content...</p>

    <section>
      <h3>Subsection</h3>
      <p>More content...</p>
    </section>
  </section>

  <section>
    <h2>Another Section</h2>
    <p>Content...</p>
  </section>
</main>

// ❌ WRONG: Skipped heading levels
<h1>Title</h1>
<h3>Jumped to h3</h3>  {/* Should be h2 */}
```

## Related Rules

- [7-2: ARIA Attributes](7-2-aria-attributes.md) - When semantics aren't enough
- [7-3: Keyboard Navigation](7-3-keyboard-navigation.md) - Interactive elements
